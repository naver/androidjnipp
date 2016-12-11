#!/usr/bin/env python2

# Copyright (C) 2015 Naver Labs. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
# OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import argparse
import collections
import string
import os, sys, time, errno
import mmap, re

import plyj.parser
import plyj.model as m

class LOG:
    @staticmethod
    def V(log):
        pass #print("LOG(V):" + log)

    @staticmethod
    def D(log):
        print("LOG(D):" + log)

    @staticmethod
    def E(log):
        print("LOG(E):" + log)

def hasModifier(declaration, name):
    for modifier in declaration.modifiers:
        if type(modifier) is not m.Annotation:
            if (modifier == name):
                return True
    return False

def getAnnotation(declaration, name):
    for modifier in declaration.modifiers:
        if type(modifier) is m.Annotation:
            if (modifier.name.value == name):
                return modifier
    return None

def hasAnnotation(declaration, name):
    modifier = getAnnotation(declaration, name)
    if modifier is not None:
        return True
    return False

def annotationValue(value):
    if type(value) is str:
        return string.replace(value, '\"', '')
    return value

def getAnnotationValue(declaration, name):
    modifier = getAnnotation(declaration, name)
    if modifier is None:
        return None
    if modifier.single_member is not None:
        return annotationValue(modifier.single_member.value)
    else:
        return getAnnotationElementValue(declaration, name, 'value')
    return None

def getAnnotationElementValue(declaration, name, element):
    modifier = getAnnotation(declaration, name)
    if modifier is None:
        return None
    for member in modifier.members:
        if member.name.value == element:
            return annotationValue(member.value.value)
    return None

def getTypeName(named_type):
    if named_type is None:
        return None
    elif type(named_type) is str:
        return named_type
    elif type(named_type.name) is str:
        return named_type.name
    else:
        return named_type.name.value

def getTypeDimensions(named_type):
    if named_type is None:
        return None
    elif type(named_type) is str:
        return 0
    else:
        return named_type.dimensions

def getInitializerValue(initializer, base_type):
    if initializer is None:
        return '\"\"' if isStringType(base_type) else '0'
    elif type(initializer) is str:
        return initializer
    elif type(initializer) is m.Literal:
        return initializer.value
    elif type(initializer) is m.Name:
        return initializer.value
    elif type(initializer) is m.Unary:
        return initializer.sign + initializer.expression.value

class Parameter:
    def __init__(self, is_final=False, base_type=None, dimensions=0, name=None):
        self.is_final = is_final
        self.base_type = base_type
        self.dimensions = dimensions
        self.name = name

def makeParameter(formal_parameter):
    parameter = Parameter()
    parameter.is_final = hasModifier(formal_parameter, 'final')
    parameter.base_type = getTypeName(formal_parameter.type)
    parameter.dimensions = getTypeDimensions(formal_parameter.type)
    parameter.name = formal_parameter.variable.name
    return parameter

def makeParameterList(declaration):
    parameters = []
    for parameter in declaration.parameters:
        parameters.append(makeParameter(parameter))
    return parameters

tab_character = '    '
natives_files_suffix = 'Natives'
managed_files_suffix = 'Managed'
any_object = '$ANYOBJECT'
cpp_string = '$CPPSTRING'

def puts(ts, *args):
    if args is not None:
        argc = 1
        for arg in args:
            ts = string.replace(ts, "%" + str(argc), arg)
            argc += 1
    return ts

wdays = ['Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat', 'Sun']
month = ['???', 'Jan', 'Feb', 'Mar', 'Apr', 'May', 'Jun', 'Jul', 'Aug', 'Sep', 'Oct', 'Nov', 'Dec']

def formatTime(tm):
    return "%s %s %02d %02d:%02d:%02d %04d" % (wdays[tm.tm_wday], month[tm.tm_mon], tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, tm.tm_year)

class SourceFile:
    def __init__(self, filepath, force_flag):
        if not os.path.isfile(filepath):
            sys.exit(-1)

        self.filepath = filepath
        self.filename = os.path.split(filepath)[-1]
        self.filename_only = string.split(self.filename, '.')[0]
        self.package = self.findPackage(filepath)
        self.last_modified = os.path.getmtime(filepath)
        self.force_flag = force_flag
        self.parsed_tree = None

    def findPackage(self, filepath):
        size = os.stat(filepath).st_size
        mtime = os.stat(filepath).st_mtime
        filename = os.path.basename(filepath)

        fp = open(filepath)
        filedata = mmap.mmap(fp.fileno(), size, access=mmap.ACCESS_READ)

        match = re.search('package (.+);', filedata)
        package = match.group(1).replace('.', '/')
        fp.close()
        return package;

    def generateSourceFilenameCommentString(self):
        return "// Generated from " + string.replace(os.path.relpath(self.filepath, __file__), '\\', '/') + "\n"

    def generateSourceFileLastModifiedTimeCommentString(self):
        return "// Last modified: " + formatTime(time.localtime(self.last_modified)) + "\n"

    def isModifiedSince(self, target_file):
        if self.force_flag:
            return True

        if not os.path.exists(target_file):
            return True

        with open(target_file, 'r') as handle:
            textline = handle.readline()
            if textline.replace('\r', '') != self.generateSourceFilenameCommentString():
                return True

            textline = handle.readline()
            if textline.replace('\r', '') != self.generateSourceFileLastModifiedTimeCommentString():
                return True

        return False

    def parsedTree(self):
        if self.parsed_tree is not None:
            return self.parsed_tree

        javaparser = plyj.parser.Parser()
        self.parsed_tree = javaparser.parse_file(self.filepath)
        return self.parsed_tree

ClassExport = collections.namedtuple('ClassExport', 'decl alias')
NativeConstructor = collections.namedtuple('NativeConstructor', 'name parameters is_abstract')
NativeDestructor = collections.namedtuple('NativeDestructor', 'name parameters is_abstract')
NativeObjectField = collections.namedtuple('NativeObjectField', 'name base_type')
NativeMethod = collections.namedtuple('NativeMethod', 'name parameters is_static is_abstract return_type')
NativeField = collections.namedtuple('NativeField', 'name initializer base_type')
NativeMethodRegistry = collections.namedtuple('NativeMethodRegistry', 'name signatures function_name')

class GeneratorBackendOverrides:
    def __init__(self):
        return

    def uniqueFileIdentifier(self, file_name, package_name):
        return '_'.join([string.replace(package_name, '.', '_'), self.internalNamespace(), file_name])

    def uniqueClassInternalIdentifier(self, file_name, package_name):
        return '_'.join([string.replace(package_name, '.', '_'), self.internalNamespace(), file_name])

    def uniqueClassExternalIdentifier(self, file_name, package_name):
        return '_'.join([string.replace(package_name, '.', '_'), self.externalNamespace(), file_name])

    def hasManaged(self):
        return False

    def commonBaseClass(self):
        return None

    def managedObjectType(self):
        return 'void*'

    def forbidOverrideNativeMethod(self):
        return False

    def abstractMethodModifier(self):
        return ''

    def abstractNativeMethodModifier(self):
        return ''

    def internalNamespace(self):
        return ""

    def externalNamespace(self):
        return ""

    def internalPassObject(self, object_type):
        return object_type

    def internalArrayObject(self, base_type):
        return base_type

    def internalTypeOfAnyObject(self):
        return 'void*'

    def externalTypeOfAnyObject(self):
        return 'void*'

    def inboundTypeCast(self):
        return ''

    def outboundTypeCast(self):
        return ''

    def internalTypeOfType(self, base_type, type_dimensions):
        return base_type

    def externalTypeOfType(self, base_type, type_dimensions):
        return base_type

    def externalTypeOfObject(self, object_type):
        return ''.join([self.externalNamespace(), '::', object_type])

class GeneratorBackend:
    class ClassAttributes:
        def __init__(self):
            self.class_type_parameters = []
            self.has_trivial_constructor = False
            self.has_native_constructors = False
            self.has_native_destructor = False
            self.native_constructors = []
            self.native_destructor = None
            self.has_abstract_method = False
            self.has_abstract_native_method = False
            self.native_object_field = None

    def __init__(self, overrides):
        self.package_name = ""
        self.package_path = []
        self.file_name = ""
        self.class_imports = {}
        self.namespace_stack = []
        self.class_name_stack = []
        self.class_attribute = None
        self.class_attribute_stack = []
        self.unknown_parameter_types = []
        self.native_fields = []
        self.template = ""
        self.indention = 0
        self.line_feed = 0
        self.overrides = overrides

    def __repr__(self):
        return self.template

    def puts(self, ts, *args):
        if ts == "":
            return

        ts = puts(ts, *args)
        ends_with_line_feed = (ts[-1] == '\n')
        max_replace = string.count(ts, '\n') - 1 if ends_with_line_feed else -1
        ts = string.replace(ts, '\n', '\n' + tab_character * self.indention, max_replace)
        if self.line_feed == 0:
            self.template += tab_character * self.indention
        self.template += ts
        if ends_with_line_feed:
            self.line_feed = 0
        else:
            self.line_feed += 1

    def INC(self):
        self.indention += 1

    def DEC(self):
        self.indention -= 1

    def EOL(self):
        self.template += '\n'
        self.line_feed = 0

    def substitute(self, key_values):
        ts = string.Template(self.template)
        return ts.safe_substitute(key_values)

    def nativePackageName(self):
        return string.replace(self.package_name, '.', '/')

    def namespacePath(self):
        return '::'.join(self.namespace_stack)

    def className(self):
        return self.class_name_stack[-1]

    def classPath(self):
        return '::'.join(self.class_name_stack)

    def internalClassExportName(self):
        return self.overrides.uniqueClassInternalIdentifier(self.file_name, self.package_name)

    def externalClassExportName(self):
        return self.overrides.uniqueClassExternalIdentifier(self.file_name, self.package_name)

    def isClassTypeParameter(self, type_name):
        return type_name in self.class_attribute.class_type_parameters

    def maybeUnknownTypeOfValue(self, base_type):
        if not isObjectType(base_type) or isAnyObjectType(base_type) or self.isClassTypeParameter(base_type):
            return False
        if base_type not in self.unknown_parameter_types:
            self.unknown_parameter_types.append(base_type)
            return True
        return False

    def resolveClassTypeParameter(self, base_type):
        return 'Object' if self.isClassTypeParameter(base_type) else base_type

    def resolveInternalType(self, base_type, type_dimensions):
        actual_type = self.resolveClassTypeParameter(base_type)
        self.maybeUnknownTypeOfValue(actual_type)
        internal_type = self.overrides.internalTypeOfType(actual_type, type_dimensions)
        if isAnyObjectType(actual_type):
            return self.overrides.internalTypeOfAnyObject()
        elif isObjectType(actual_type):
            internal_type = ''.join([self.overrides.internalNamespace(), '::', internal_type])
        return internal_type

    def resolveExternalType(self, base_type, type_dimensions):
        actual_type = self.resolveClassTypeParameter(base_type)
        self.maybeUnknownTypeOfValue(actual_type)
        external_type = self.overrides.externalTypeOfType(actual_type, type_dimensions)
        if isAnyObjectType(actual_type):
            return self.overrides.externalTypeOfAnyObject()
        elif isObjectType(actual_type):
            external_type = self.overrides.externalTypeOfObject(external_type)
        return external_type

    def resolvePassType(self, base_type, type_dimensions, as_reference):
        internal_type = self.resolveInternalType(base_type, type_dimensions)
        if isObjectType(base_type):
            self.maybeUnknownTypeOfValue(base_type)
            native_type = self.overrides.internalPassObject(internal_type)
        else:
            use_reference = as_reference and not isPrimitiveType(base_type) and type_dimensions == 0
            native_type = ''.join(["const ", internal_type, '&']) if use_reference else internal_type
        if type_dimensions > 0:
            native_type = self.overrides.internalArrayObject(native_type)
            if as_reference:
                native_type = ''.join(["const ", native_type, '&'])
        return native_type

    def surroundWithCast(self, base_type, type_dimensions, term, is_outbound=True):
        ts = self.overrides.outboundTypeCast() if is_outbound else self.overrides.inboundTypeCast()
        if not isPrimitiveType(base_type) and not isStringType(base_type):
            if is_outbound:
                ts = ''.join([ts, '<', self.resolveExternalType(base_type, type_dimensions), '>'])
            else:
                ts = ''.join([ts, '<', self.resolveInternalType(base_type, type_dimensions), '>'])
        return ts if term is None else ''.join([ts, '(', term, ')'])

    def imported(self, class_name):
        if class_name in self.class_imports:
            return True
        else:
            return False

    def importClassPackage(self, class_name):
        if not self.imported(class_name):
            return self.package_path
        return self.class_imports[class_name]

    def importClassDeclaration(self, class_name, namespace):
        return ' '.join(['namespace', ' { namespace '.join(self.importClassPackage(class_name) + [namespace]), '{', 'class', class_name, ';', '}' * (len(self.importClassPackage(class_name)) + 1)])

    def importClassInclude(self, class_name, namespace):
        return ' '.join(["#include", ''.join(['<', '/'.join(self.importClassPackage(class_name)), '/', class_name, namespace, "Base.h", '>'])])

    def importClassPath(self, class_name, namespace):
        return '::'.join(self.importClassPackage(class_name) + [namespace, class_name])

    def importClassTypedef(self, class_name, namespace):
        return '_'.join(self.importClassPackage(class_name) + [namespace, class_name])

    def buildArgumentList(self, parameters, is_outbound):
        arguments = []
        if len(parameters) > 0:
            for parameter in parameters:
                arguments.append(self.surroundWithCast(parameter.base_type, parameter.dimensions, parameter.name, is_outbound))
        return arguments

    def buildMethodReturn(self, return_type):
        return self.resolvePassType(getTypeName(return_type), getTypeDimensions(return_type), False)

    def buildParameter(self, parameter, call_by_reference):
        parameter_name = "" if parameter.name == "" else ' ' + parameter.name
        return self.resolvePassType(parameter.base_type, parameter.dimensions, call_by_reference) + parameter_name

    def buildParameters(self, parameters, call_by_reference):
        if len(parameters) > 0:
            first_parameter = parameters[0]
            next_parameters = parameters[1:]
            result = self.buildParameter(first_parameter, call_by_reference)
            for parameter in next_parameters:
                result += '\n'
                result += tab_character
                result += ', '
                result += self.buildParameter(parameter, call_by_reference)
            return result
        return ""

    def buildArgument(self, parameter, is_outbound):
        return self.surroundWithCast(parameter.base_type, parameter.dimensions, parameter.name) if is_outbound else parameter.name

    def buildArguments(self, parameters, is_outbound):
        if len(parameters) > 0:
            first_parameter = parameters[0]
            next_parameters = parameters[1:]
            result = self.buildArgument(first_parameter, is_outbound)
            for parameter in next_parameters:
                result += '\n'
                result += tab_character
                result += ', '
                result += self.buildArgument(parameter, is_outbound)
            return result
        return ""

    def processSourceFile(self, source_file):
        self.puts(source_file.generateSourceFilenameCommentString())
        self.puts(source_file.generateSourceFileLastModifiedTimeCommentString())
        self.puts("// THIS FILE IS AUTO-GENERATED. DO NOT MODIFY.\n")

    def processFileHeader(self, name):
        LOG.V('processFileHeader: ' + name)
        self.file_name = name
        package_parts = string.split(self.package_name, '.')

    def processImport(self, name):
        LOG.V('processImport: ' + name)
        name_parts = string.split(name, '.')
        self.class_imports[name_parts[-1]] = name_parts[:-1]

    def processPackage(self, name):
        LOG.V('processPackage: ' + name)
        self.package_name = name
        self.package_path = string.split(name, '.')

    def processNamespaceBegin(self, name):
        LOG.V('processNamespaceBegin: ' + name)
        self.namespace_stack.append(name)

    def processNamespaceEnd(self, name):
        LOG.V('processNamespaceEnd: ' + name)
        self.namespace_stack.pop()

    def processClassBegin(self, name, native_export_macro, extends, class_type_parameters, has_trivial_constructor, has_native_constructors):
        LOG.V('processClassBegin: ' + name)
        self.class_attribute_stack.append(self.class_attribute)
        self.class_name_stack.append(name)
        self.class_attribute = GeneratorBackend.ClassAttributes()
        self.class_attribute.class_type_parameters = class_type_parameters
        self.class_attribute.has_trivial_constructor = has_trivial_constructor
        self.class_attribute.has_native_constructors = has_native_constructors

    def processClassEnd(self, name):
        LOG.V('processClassEnd: ' + name)
        self.template = string.Template(self.template).safe_substitute({
                                             'CLASS_NAME' : self.className(),
                                             'CLASS_PATH' : self.classPath(),
                                             'PASS_CLASS' : self.overrides.internalPassObject(self.classPath()),
                                             })
        if self.class_attribute.native_object_field is not None:
            assert(self.class_attribute.has_native_destructor)
        self.class_attribute = self.class_attribute_stack.pop()
        self.class_name_stack.pop()

    def processConstructor(self, called_by_native, parameters):
        LOG.V('processConstructor: ')

    def processNativeConstructor(self, name, parameters, is_abstract):
        LOG.V('processNativeConstructor: ')
        self.class_attribute.native_constructors.append(NativeConstructor(name, parameters, is_abstract))

    def processNativeDestructor(self, name, parameters, is_abstract):
        LOG.V('processNativeDestructor: ')
        self.class_attribute.has_native_destructor = True
        assert(self.class_attribute.native_destructor == None)
        self.class_attribute.native_destructor = NativeDestructor(name, parameters, is_abstract)

    def processNativeObjectField(self, name, base_type):
        LOG.V('processNativeObjectField: ' + name)
        assert(self.class_attribute.native_object_field is None)
        self.class_attribute.native_object_field = NativeObjectField(name, base_type)

    def processMethod(self, called_by_native, is_static, is_abstract, return_type, name, parameters):
        LOG.V('processMethod: ' + name)
        if is_abstract and not self.class_attribute.has_abstract_method:
            self.class_attribute.has_abstract_method = True

    def processNativeMethod(self, is_static, is_abstract, return_type, name, parameters):
        LOG.V('processNativeMethod: ' + name)
        if is_abstract and not self.class_attribute.has_abstract_native_method:
            self.has_abstract_native_method = False

    def processField(self, accessed_by_native, is_static, is_final, base_type, dimensions, initializer, name):
        LOG.V('processField: ' + name)
        if accessed_by_native and (not is_static and not is_final or initializer is None):
            self.native_fields.append(NativeField(name, initializer, base_type))

    def processSupplement(self, supplement_for_managed, supplement_for_natives):
        LOG.V('processSupplement: ')

    def processEOF(self):
        LOG.V('processEOF: ')
        self.template = string.Template(self.template).safe_substitute({
                                             'INTERNAL_NAMESPACE' : self.overrides.internalNamespace(),
                                             'EXTERNAL_NAMESPACE' : self.overrides.externalNamespace(),
                                             })
        self.template = string.replace(self.template, '\b ', '')
        self.template = string.replace(self.template, '\b', '')

primitive_types = ['void', 'char', 'int', 'long', 'short', 'byte', 'float', 'double', 'boolean']

def isPrimitiveType(typename):
    if typename in primitive_types:
        return True
    return False

def isStringType(typename):
    if typename == 'String':
        return True
    return False

def isAnyObjectType(typename):
    if typename == 'Object':
        return True
    return False

def isObjectType(typename):
    return isAnyObjectType(typename) or (not isPrimitiveType(typename) and not isStringType(typename))

class TypeMap:
    def __init__(self):
        self.type_map = {}

    def defaultType(self, typename):
        return typename

    def modifyTypeForDimensions(self, typename, dimensions):
        return typename

    def mapType(self, typename, native_type_for_dimensions):
        self.type_map[typename] = native_type_for_dimensions

    def mappedType(self, typename, dimensions):
        if typename not in self.type_map:
            return self.modifyTypeForDimensions(self.defaultType(typename), dimensions)
        if len(self.type_map[typename]) < dimensions + 1:
            return self.modifyTypeForDimensions(self.type_map[typename][0], dimensions)
        return self.type_map[typename][dimensions]

class CPPTypeMap(TypeMap):
    def __init__(self):
        TypeMap.__init__(self)
        self.mapType('void',    ['void'])
        self.mapType('int',     ['int32_t'])
        self.mapType('long',    ['int64_t'])
        self.mapType('short',   ['int16_t'])
        self.mapType('byte',    ['int8_t'])
        self.mapType('float',   ['float'])
        self.mapType('double',  ['double'])
        self.mapType('boolean', ['bool'])
        self.mapType('Object',  [any_object])
        self.mapType('String',  [cpp_string])

cpp_type_map = CPPTypeMap()

def CPP_TYPE(typename, dimensions):
    return cpp_type_map.mappedType(typename, dimensions)

class CallTypeMap(TypeMap):
    def __init__(self):
        TypeMap.__init__(self)
        self.mapType('void',    ["Void"])
        self.mapType('int',     ["Int"])
        self.mapType('long',    ["Long"])
        self.mapType('short',   ["Short"])
        self.mapType('byte',    ["Byte"])
        self.mapType('float',   ["Float"])
        self.mapType('double',  ["Double"])
        self.mapType('boolean', ["Boolean"])
        self.mapType('void*',   ['Object'])
        self.mapType('String',  ["String"])

    def defaultType(self, typename):
        return 'Object'

    def modifyTypeForDimensions(self, typename, dimensions):
        return 'Object' if dimensions > 0 else typename

call_type_map = CallTypeMap()

def CALL_TYPE(typename, dimensions):
    return call_type_map.mappedType(typename, dimensions)

class JNITypeMap(TypeMap):
    def __init__(self):
        TypeMap.__init__(self)
        self.mapType('void',    ['void'])
        self.mapType('int',     ['jint'])
        self.mapType('long',    ['jlong'])
        self.mapType('short',   ['jshort'])
        self.mapType('byte',    ['jbyte'])
        self.mapType('float',   ['jfloat'])
        self.mapType('double',  ['jdouble'])
        self.mapType('boolean', ['jboolean'])
        self.mapType('Object',  ['jobject'])
        self.mapType('String',  ['jstring'])

    def defaultType(self, typename):
        return 'jobject'

    def modifyTypeForDimensions(self, typename, dimensions):
        return typename + 'Array' if dimensions > 0 else typename

jni_type_map = JNITypeMap()

def JNI_TYPE(typename, dimensions):
    return jni_type_map.mappedType(typename, dimensions)

class JNISignatureMap(TypeMap):
    def __init__(self):
        TypeMap.__init__(self)
        self.mapType('void',    ['V'])
        self.mapType('int',     ['I'])
        self.mapType('long',    ['J'])
        self.mapType('short',   ['S'])
        self.mapType('byte',    ['B'])
        self.mapType('float',   ['F'])
        self.mapType('double',  ['D'])
        self.mapType('boolean', ['Z'])
        self.mapType('Object',  ['Ljava/lang/Object;'])
        self.mapType('String',  ['Ljava/lang/String;'])

    def defaultType(self, typename):
        return "L\" PACKAGE_NAME \"/" + typename + ";"

    def modifyTypeForDimensions(self, typename, dimensions):
        return '[' * dimensions + typename if dimensions > 0 else typename

jni_signature_map = JNISignatureMap()

def JNI_SIGNATURE(typename, dimensions):
    return jni_signature_map.mappedType(typename, dimensions)

class GeneratorFrontend:
    def __init__(self):
        self.backend = None

    def generateFromSourceFile(self, source_file, backend):
        self.backend = backend

        tree = source_file.parsedTree()
        self.backend.processSourceFile(source_file)
        self.backend.processPackage(tree.package_declaration.name.value)
        self.backend.processFileHeader(tree.type_declarations[0].name)
        for import_declaration in tree.import_declarations:
            self.backend.processImport(import_declaration.name.value)
        for type_declaration in tree.type_declarations:
            self.processClass(type_declaration)
        self.backend.processEOF()

    def processNamespacesBegin(self, path):
        words = string.split(path, '.')
        for word in words:
            self.backend.processNamespaceBegin(word)

    def processNamespacesEnd(self, path):
        words = string.split(path, '.')
        for word in reversed(words):
            self.backend.processNamespaceEnd(word)

    def processMethod(self, called_by_native, is_static, is_abstract, is_native, return_type, name, parameters):
        self.backend.processNativeMethod(is_static, is_abstract, return_type, name, parameters) if is_native else self.backend.processMethod(called_by_native, is_static, is_abstract, return_type, name, parameters)

    def processClass(self, type_declaration):
        if not hasAnnotation(type_declaration, 'NativeNamespace'):
            return

        namespace_path = getAnnotationValue(type_declaration, 'NativeNamespace')
        if namespace_path is None:
            return

        class_type_parameters = []

        for type_parameter in type_declaration.type_parameters:
            class_type_parameters.append(type_parameter.name)

        def hasNativeConstructors():
            for declaration in type_declaration.body:
                if type(declaration) is m.MethodDeclaration:
                    if hasAnnotation(declaration, 'NativeConstructor') or hasAnnotation(declaration, 'NativeDestructor'):
                        return True
            return False

        def hasTrivialConstructor():
            for declaration in type_declaration.body:
                if type(declaration) is m.ConstructorDeclaration:
                    return False
            return True

        self.processNamespacesBegin(namespace_path)

        has_trivial_constructor = hasTrivialConstructor()

        native_export_macro = getAnnotationValue(type_declaration, 'NativeExportMacro') if hasAnnotation(type_declaration, 'NativeExportMacro') else None

        self.backend.processClassBegin(type_declaration.name, native_export_macro, getTypeName(type_declaration.extends),
                                       class_type_parameters, has_trivial_constructor, hasNativeConstructors())

        if has_trivial_constructor:
            self.backend.processConstructor(True, [])

        for declaration in type_declaration.body:
            if type(declaration) is m.ConstructorDeclaration:
                self.backend.processConstructor(hasAnnotation(declaration, 'CalledByNative')
                                           , makeParameterList(declaration))
            elif type(declaration) is m.MethodDeclaration:
                if hasAnnotation(declaration, 'NativeConstructor'):
                    assert(not hasModifier(declaration, 'static'))
                    assert(declaration.return_type == 'void')
                    self.backend.processNativeConstructor(declaration.name
                                                          , makeParameterList(declaration)
                                                          , hasAnnotation(declaration, 'AbstractMethod'))
                elif hasAnnotation(declaration, 'NativeDestructor'):
                    assert(not hasModifier(declaration, 'static'))
                    assert(declaration.return_type == 'void')
                    self.backend.processNativeDestructor(declaration.name
                                                         , makeParameterList(declaration)
                                                         , hasAnnotation(declaration, 'AbstractMethod'))
                else:
                    self.processMethod(hasAnnotation(declaration, 'CalledByNative')
                                          , hasModifier(declaration, 'static')
                                          , hasAnnotation(declaration, 'AbstractMethod')
                                          , hasModifier(declaration, 'native')
                                          , declaration.return_type, declaration.name
                                          , makeParameterList(declaration))
            elif type(declaration) is m.FieldDeclaration:
                if hasAnnotation(declaration, 'NativeObjectField'):
                    assert(not hasModifier(declaration, 'static'))
                    assert(not hasModifier(declaration, 'final'))
                    self.backend.processNativeObjectField(declaration.variable_declarators[0].variable.name, getTypeName(declaration.type))
                else:
                    self.backend.processField(hasAnnotation(declaration, 'AccessedByNative')
                                         , hasModifier(declaration, 'static')
                                         , hasModifier(declaration, 'final')
                                         , getTypeName(declaration.type)
                                         , declaration.variable_declarators[0].variable.dimensions
                                         , declaration.variable_declarators[0].initializer
                                         , declaration.variable_declarators[0].variable.name)
            elif type(declaration) is m.ClassDeclaration:
                self.processClass(declaration)
            else:
                continue

            self.backend.processSupplement(getAnnotationValue(declaration, 'SupplementForManaged')
                                   , getAnnotationValue(declaration, 'SupplementForNatives'))

        self.backend.processClassEnd(type_declaration.name)

        self.processNamespacesEnd(namespace_path)

class Visibility:
    UNKNOWN = -1
    PRIVATE = 0
    PROTECTED = 1
    PUBLIC = 2

class HeaderGeneratorBackend(GeneratorBackend):
    def __init__(self, overrides):
        GeneratorBackend.__init__(self, overrides)
        self.visibility = Visibility.UNKNOWN

    visibility_keywords = { Visibility.PRIVATE : 'private:', Visibility.PROTECTED : 'protected:', Visibility.PUBLIC : 'public:' }

    def setVisibility(self, visibility):
        if visibility is not self.visibility:
            if visibility not in self.visibility_keywords:
                self.visibility = Visibility.UNKNOWN
                return
            self.puts(self.visibility_keywords[visibility])
            self.visibility = visibility
            self.EOL()

    def processFileHeader(self, name):
        GeneratorBackend.processFileHeader(self, name)
        ts = (
        "#ifndef ANDROIDJNI_GENERATED_%1_h",
        "#define ANDROIDJNI_GENERATED_%1_h",
        "")
        self.puts('\n'.join(ts), self.overrides.uniqueFileIdentifier(self.file_name, self.package_name))
        self.EOL()

    def processNamespaceBegin(self, name):
        GeneratorBackend.processNamespaceBegin(self, name)
        self.puts("namespace %1 {\n", name)

    def processNamespaceEnd(self, name):
        GeneratorBackend.processNamespaceEnd(self, name)
        self.puts("} // namespace %1\n", name)

    def processEOF(self):
        GeneratorBackend.processEOF(self)
        self.EOL()
        self.puts("#endif // End of File\n")

class InterfaceHeaderGeneratorBackend(HeaderGeneratorBackend):
    def __init__(self, overrides):
        HeaderGeneratorBackend.__init__(self, overrides)

    def annotateImplementManaged(self):
        return

    def annotateImplementNatives(self):
        return

    def defineConstructorStubs(self):
        return

    def defineFinalField(self, is_static, typename, dimensions, initializer, name):
        ts = (
        "$EXPORT_MACRO${FIELD_SPECIFIER}const $FIELD_TYPE $FIELD_NAME$FIELD_INITIALIZER;",
        "")
        ts = string.Template('\n'.join(ts)).safe_substitute({
                                             'EXPORT_MACRO' : 'CLASS_EXPORT ' if not isPrimitiveType(typename) else '',
                                             'FIELD_SPECIFIER' : 'static ' if is_static else '',
                                             'FIELD_TYPE' : self.buildParameter(Parameter(False, typename, dimensions, ""), False),
                                             'FIELD_NAME' : name,
                                             'FIELD_INITIALIZER' : (' = ' + getInitializerValue(initializer, typename)) if initializer is not None and isPrimitiveType(typename) else '',
                                             })
        self.puts(ts)

    def defineField(self, is_static, typename, dimensions, initializer, name):
        ts = (
        "$EXPORT_MACRO$FIELD_SPECIFIER$FIELD_TYPE $FIELD_NAME;",
        "")
        ts = string.Template('\n'.join(ts)).safe_substitute({
                                             'EXPORT_MACRO' : 'CLASS_EXPORT ' if is_static else '',
                                             'FIELD_SPECIFIER' : 'static ' if is_static else '',
                                             'FIELD_TYPE' : self.buildParameter(Parameter(False, typename, dimensions, ""), False),
                                             'FIELD_NAME' : name,
                                             })
        self.puts(ts)

    def processClassBegin(self, name, native_export_macro, extends, class_type_parameters, has_trivial_constructor, has_native_constructors):
        HeaderGeneratorBackend.processClassBegin(self, name, native_export_macro, extends, class_type_parameters, has_trivial_constructor, has_native_constructors)

        extends = None  # Disable extends
        base_class = extends if extends is not None else self.overrides.commonBaseClass()

        if extends is not None and extends not in self.unknown_parameter_types:
            self.unknown_parameter_types.append(extends)

        ts = (
        "namespace $EXTERNAL_NAMESPACE {",
        "class $CLASS_NAME;",
        "} // namespace $EXTERNAL_NAMESPACE",
        "",
        "namespace $INTERNAL_NAMESPACE {",
        "$CLASS_FORWARD_DECLARATIONS",
        "")
        self.puts('\n'.join(ts))
        ts = (
        "class $CLASS_NAME%1 {",
        "#define CLASS_EXPORT%2",
        "")
        self.puts('\n'.join(ts),
                  '' if base_class is None else ' '.join([' :', 'public', base_class]),
                  '' if native_export_macro is None else ' '.join(['', native_export_macro]))

        self.setVisibility(Visibility.PUBLIC)
        self.INC()
        ts = (
        "class NativeBindings;",
        "friend class NativeBindings;",
        "friend class $EXTERNAL_NAMESPACE::$CLASS_NAME;",
        "",
        "virtual ~$CLASS_NAME() {$INVOKE_NATIVE_DESTRUCTOR}",
        "")
        self.puts('\n'.join(ts))
        self.DEC()
        self.EOL()

    def processClassEnd(self, name):
        self.setVisibility(Visibility.UNKNOWN)
        self.setVisibility(Visibility.PUBLIC)
        ts = (
        "// TODO: DEFINE PRIVATE CLASS(IF NEEDED)",
        "class Private { public: virtual ~Private() { } };"
        "")
        self.INC()
        self.puts('\n'.join(ts))
        self.DEC()
        self.EOL()
        self.EOL()

        self.INC()
        ts = (
        "// NOTE: SHOULD BE CALLED DURING MODULE INITIALIZATION",
        "CLASS_EXPORT static bool registerClass();",
        "")
        self.puts('\n'.join(ts))
        self.DEC()
        self.EOL()

        self.setVisibility(Visibility.PROTECTED)
        self.INC()
        self.puts("CLASS_EXPORT $CLASS_NAME();\n")
        self.EOL()
        self.DEC()

        self.INC()
        self.defineConstructorStubs()

        ts = (
        "$AUTO_PTR<Private> m_private;",
        "")
        self.puts('\n'.join(ts))
        self.DEC()

        self.puts("}; // class $CLASS_NAME\n")
        self.EOL()
        self.puts("#undef CLASS_EXPORT\n")
        self.EOL()
        self.puts("} // namespace $INTERNAL_NAMESPACE")
        self.EOL()

        self.template = self.template.replace("$INVOKE_NATIVE_DESTRUCTOR", ' ')
        GeneratorBackend.processClassEnd(self, name)

    def processConstructor(self, called_by_native, parameters):
        HeaderGeneratorBackend.processConstructor(self, called_by_native, parameters)
        if called_by_native is False:
            return

        self.setVisibility(Visibility.PUBLIC)
        self.INC()
        ts = (
        "CLASS_EXPORT static $PASS_CLASS create(%1);",
        "")
        self.puts('\n'.join(ts), self.buildParameters(parameters, True))
        self.DEC()
        self.EOL()

    def processNativeObjectField(self, name, base_type):
        HeaderGeneratorBackend.processNativeObjectField(self, name, base_type)
        self.processField(True, False, False, base_type, 0, None, name)

    def processMethod(self, called_by_native, is_static, is_abstract, return_type, name, parameters):
        HeaderGeneratorBackend.processMethod(self, called_by_native, is_static, is_abstract, return_type, name, parameters)
        if called_by_native is False:
            return

        self.setVisibility(Visibility.PUBLIC)
        self.INC()
        self.annotateImplementManaged()
        ts = (
        "CLASS_EXPORT $METHOD_SPECIFIER $METHOD_RETURN_TYPE $METHOD_NAME($METHOD_PARAMETERS)$METHOD_MODIFIER;",
        "")
        ts = string.Template('\n'.join(ts)).safe_substitute({
                                             'METHOD_SPECIFIER' : 'static' if is_static else 'virtual',
                                             'METHOD_RETURN_TYPE' : self.buildMethodReturn(return_type),
                                             'METHOD_NAME' : name,
                                             'METHOD_PARAMETERS' : self.buildParameters(parameters, True),
                                             'METHOD_MODIFIER' : self.overrides.abstractMethodModifier() if not is_static and is_abstract else '',
                                             })
        self.puts(ts)
        self.DEC()
        self.EOL()

    def processNativeMethod(self, is_static, is_abstract, return_type, name, parameters):
        HeaderGeneratorBackend.processNativeMethod(self, is_static, is_abstract, return_type, name, parameters)

        self.setVisibility(Visibility.PUBLIC)
        self.INC()
        self.annotateImplementNatives()
        ts = (
        "CLASS_EXPORT $METHOD_SPECIFIER $METHOD_RETURN_TYPE $METHOD_NAME($METHOD_PARAMETERS)$METHOD_MODIFIER;",
        "")
        ts = string.Template('\n'.join(ts)).safe_substitute({
                                             'METHOD_SPECIFIER' : 'static' if is_static else 'virtual' if not self.overrides.forbidOverrideNativeMethod() else '\b',
                                             'METHOD_RETURN_TYPE' : self.buildMethodReturn(return_type),
                                             'METHOD_NAME' : name,
                                             'METHOD_PARAMETERS' : self.buildParameters(parameters, True),
                                             'METHOD_MODIFIER' : self.overrides.abstractNativeMethodModifier() if not is_static and is_abstract else '',
                                             })
        self.puts(ts)
        self.DEC()
        self.EOL()

    def processField(self, accessed_by_native, is_static, is_final, base_type, dimensions, initializer, name):
        HeaderGeneratorBackend.processField(self, accessed_by_native, is_static, is_final, base_type, dimensions, initializer, name)
        if accessed_by_native is False:
            return

        self.setVisibility(Visibility.PUBLIC)
        self.INC()
        typename = getTypeName(base_type)
        if is_final and initializer is not None:
            self.defineFinalField(is_static, typename, dimensions, initializer, name)
        else:
            self.defineField(is_static, typename, dimensions, initializer, name)
        self.DEC()
        self.EOL()

    def processEOF(self):
        class_import_headers = ""
        forward_declarations = ""
        if len(self.unknown_parameter_types) > 0:
            forward_declarations += '\n'
            for unknown_type in self.unknown_parameter_types:
                class_import_headers += '\n'.join([self.importClassDeclaration(unknown_type, self.overrides.internalNamespace()),
                                                   ' '.join(['typedef', 'class', self.importClassPath(unknown_type, self.overrides.internalNamespace()), self.importClassTypedef(unknown_type, self.overrides.internalNamespace()), ";\n"])
                                                  ]) if self.imported(unknown_type) is not None else ''
                forward_declarations += ''.join(["typedef ", self.importClassTypedef(unknown_type, self.overrides.internalNamespace()), ' ', unknown_type, ";\n"]) if self.imported(unknown_type) is not None \
                else ''.join(["class ", unknown_type, ";\n"])
        self.template = self.template.replace("$CLASS_IMPORT_HEADERS", class_import_headers)
        self.template = self.template.replace("$CLASS_FORWARD_DECLARATIONS", forward_declarations)
        HeaderGeneratorBackend.processEOF(self)

class NativesHeaderGeneratorBackendOverrides(GeneratorBackendOverrides):
    def __init__(self):
        GeneratorBackendOverrides.__init__(self)

    def commonBaseClass(self):
        return 'JNI::NativeObject'

    def abstractNativeMethodModifier(self):
        return ' = 0'

    def internalNamespace(self):
        return natives_files_suffix

    def externalNamespace(self):
        return managed_files_suffix

    def internalPassObject(self, object_type):
        return '$LOCAL_REF<$T>'.replace('$T', object_type)

    def internalArrayObject(self, base_type):
        return 'JNI::PassArray<$T>'.replace('$T', base_type)

    def internalTypeOfAnyObject(self):
        return 'JNI::AnyObject'

    def externalTypeOfAnyObject(self):
        return 'void'

    def inboundTypeCast(self):
        return 'JNI::toNative'

    def outboundTypeCast(self):
        return 'JNI::toManaged'

    def internalTypeOfType(self, base_type, type_dimensions):
        return CPP_TYPE(base_type, type_dimensions)

    def externalTypeOfType(self, base_type, type_dimensions):
        return CPP_TYPE(base_type, type_dimensions)

class NativesHeaderGeneratorBackend(InterfaceHeaderGeneratorBackend):
    def __init__(self):
        InterfaceHeaderGeneratorBackend.__init__(self, NativesHeaderGeneratorBackendOverrides())

    def annotateImplementNatives(self):
        self.puts("// TODO: IMPLEMENT")
        self.EOL()

    def defineConstructorStubs(self):
        for native_constructor in self.class_attribute.native_constructors:
            self.annotateImplementNatives()
            ts = (
            "static $CLASS_NAME* %1(%2);",
            "")
            self.puts('\n'.join(ts), native_constructor.name, self.buildParameters(native_constructor.parameters, False))
            self.EOL()

    def defineField(self, is_static, typename, dimensions, initializer, name):
        ts = (
        "$FIELD_SPECIFIER$FIELD_MACRO($FIELD_NAME, $FIELD_PARAMETER);",
        "")
        ts = string.Template('\n'.join(ts)).safe_substitute({
                                             'FIELD_SPECIFIER' : 'STATIC_' if is_static else '',
                                             'FIELD_MACRO' : 'FIELD_INTERFACE',
                                             'FIELD_NAME' : name,
                                             'FIELD_PARAMETER' : self.buildParameter(Parameter(False, typename, dimensions, ""), True),
                                             })
        self.puts(ts)

    def processFileHeader(self, name):
        InterfaceHeaderGeneratorBackend.processFileHeader(self, name)
        self.puts("$COMMON_INCLUDES\n")
        self.puts("$CLASS_IMPORT_HEADERS")
        self.EOL()

    def processClassBegin(self, name, native_export_macro, extends, class_type_parameters, has_trivial_constructor, has_native_constructors):
        InterfaceHeaderGeneratorBackend.processClassBegin(self, name, native_export_macro, extends, class_type_parameters, has_trivial_constructor, has_native_constructors)

        self.INC()
        if not self.class_attribute.has_native_constructors:
            self.annotateImplementNatives()
            self.puts("static $CLASS_PATH* CTOR();")
            self.EOL()
            self.EOL()
        ts = (
        "static $LOCAL_REF<$CLASS_PATH> fromRef(JNI::ref_t);",
        "static $LOCAL_REF<$CLASS_PATH> fromPtr(std::shared_ptr<$EXTERNAL_NAMESPACE::$CLASS_PATH>&);",
        "")
        self.puts('\n'.join(ts))
        self.DEC()
        self.EOL()

    def processSupplement(self, supplement_for_managed, supplement_for_natives):
        if supplement_for_natives is None:
            return
        ts = (
        "// NOTE: SUPPLEMENTAL, IMPLEMENT IF NEEDED",
        supplement_for_natives,
        "")
        self.INC()
        self.puts('\n'.join(ts))
        self.DEC()
        self.EOL()

class ManagedHeaderGeneratorBackendOverrides(GeneratorBackendOverrides):
    def __init__(self):
        GeneratorBackendOverrides.__init__(self)

    def forbidOverrideNativeMethod(self):
        return True

    def abstractMethodModifier(self):
        return ' = 0'

    def internalNamespace(self):
        return managed_files_suffix

    def externalNamespace(self):
        return natives_files_suffix

    def internalPassObject(self, object_type):
        return 'std::shared_ptr<$T>'.replace('$T', object_type)

    def internalArrayObject(self, base_type):
        return 'std::vector<$T>'.replace('$T', base_type)

    def internalTypeOfAnyObject(self):
        return 'void'

    def externalTypeOfAnyObject(self):
        return 'JNI::AnyObject'

    def inboundTypeCast(self):
        return 'JNI::toManaged'

    def outboundTypeCast(self):
        return 'JNI::toNative'

    def internalTypeOfType(self, base_type, type_dimensions):
        return CPP_TYPE(base_type, type_dimensions)

    def externalTypeOfType(self, base_type, type_dimensions):
        return CPP_TYPE(base_type, type_dimensions)

class ManagedHeaderGeneratorBackend(InterfaceHeaderGeneratorBackend):
    def __init__(self):
        InterfaceHeaderGeneratorBackend.__init__(self, ManagedHeaderGeneratorBackendOverrides())
        self.constructors = []

    def annotateImplementManaged(self):
        self.puts("// TODO: IMPLEMENT")
        self.EOL()

    def defineConstructorStubs(self):
        self.puts("CLASS_EXPORT static std::shared_ptr<$CLASS_NAME> create(std::function<$CLASS_NAME* ()>, std::function<void ($CLASS_NAME*)>);\n")

        for constructor in self.constructors:
            self.EOL()
            self.annotateImplementManaged()
            self.puts("CLASS_EXPORT virtual void INIT(%1);\n", self.buildParameters(constructor, True))
        self.EOL()

    def processClassBegin(self, name, native_export_macro, extends, class_type_parameters, has_trivial_constructor, has_native_constructors):
        InterfaceHeaderGeneratorBackend.processClassBegin(self, name, native_export_macro, extends, class_type_parameters, has_trivial_constructor, has_native_constructors)

        ts = (
        "template<typename T, typename... Args> static inline std::shared_ptr<T> create(Args&&... arguments)",
        "{",
        "    static_assert(std::is_base_of<$CLASS_NAME, T>::value, \"Type T is not a kind of $CLASS_NAME.\");",
        "    std::shared_ptr<$CLASS_NAME> uninitialized = create([=] () { return new T(); }, [&] ($CLASS_NAME* ptr) { static_cast<T*>(ptr)->INIT(arguments...); });",
        "    return std::static_pointer_cast<T>(uninitialized);",
        "}",
        "",
        "template<typename T> static inline void runtimeLink()",
        "{",
        "    static_assert(std::is_base_of<$CLASS_NAME, T>::value, \"Type T is not a kind of $CLASS_NAME.\");",
        "    overrideCTOR([=] () { return new T(); });",
        "}",
        "")
        self.INC()
        self.puts('\n'.join(ts))
        self.DEC()
        self.EOL()

    def processFileHeader(self, name):
        InterfaceHeaderGeneratorBackend.processFileHeader(self, name)
        ts = (
        "#include <functional>",
        "#include <map>",
        "#include <memory>",
        "#include <string>",
        "#include <vector>",
        "$CLASS_IMPORT_HEADERS")
        self.puts('\n'.join(ts))
        self.EOL()

    def processClassEnd(self, name):
        self.template = self.template.replace("$INVOKE_NATIVE_DESTRUCTOR",
                                              ''.join([' ', self.class_attribute.native_destructor.name, '(); '])
                                              if self.class_attribute.native_destructor is not None else ' ')
        self.setVisibility(Visibility.UNKNOWN)
        self.setVisibility(Visibility.PRIVATE)
        self.INC()
        ts = (
        "static $CLASS_NAME* CTOR();\n",
        "// NOTE: OVERRIDE DEFAULT CONSTRUCTOR IF CLASS IS BEING REDEFINED USING INHERITANCE",
        "CLASS_EXPORT static void overrideCTOR(std::function<$CLASS_NAME* ()>);",
        "")
        self.puts('\n'.join(ts))
        self.DEC()
        self.EOL()

        InterfaceHeaderGeneratorBackend.processClassEnd(self, name)

    def processConstructor(self, called_by_native, parameters):
        InterfaceHeaderGeneratorBackend.processConstructor(self, called_by_native, parameters)
        if not called_by_native:
            return

        self.constructors.append(parameters)

    def processMethod(self, called_by_native, is_static, is_abstract, return_type, name, parameters):
        InterfaceHeaderGeneratorBackend.processMethod(self, called_by_native or is_abstract, is_static, is_abstract, return_type, name, parameters)

    def processNativeConstructor(self, name, parameters, is_abstract):
        InterfaceHeaderGeneratorBackend.processNativeConstructor(self, name, parameters, is_abstract)
        self.processNativeMethod(False, is_abstract, 'void', name, parameters)

    def processNativeDestructor(self, name, parameters, is_abstract):
        InterfaceHeaderGeneratorBackend.processNativeDestructor(self, name, parameters, is_abstract)
        self.processNativeMethod(False, is_abstract, 'void', name, parameters)

    def processSupplement(self, supplement_for_managed, supplement_for_natives):
        if supplement_for_managed is None:
            return
        ts = (
        "// NOTE: SUPPLEMENTAL, IMPLEMENT IF NEEDED",
        supplement_for_managed,
        "")
        self.INC()
        self.puts('\n'.join(ts))
        self.DEC()
        self.EOL()

class StubGeneratorBackend(GeneratorBackend):
    def __init__(self, overrides):
        GeneratorBackend.__init__(self, overrides)
        self.pending_native_methods = []

    def implementDefaultConstructor(self):
        return

    def implementRegisterClass(self):
        return

    def implementFieldAccess(self, field_name, is_static, this_reference, base_type, dimensions, get_or_set):
        return

    def implementNativeConstructor(self, name, parameters):
        return

    def implementNativeDestructor(self, name):
        return

    def implementNativeBinding(self, is_static, is_abstract, return_type, name, parameters):
        return

    def implementConstruction(self, called_by_native, parameters):
        return

    def implementMethodInvocation(self, function_name, is_static, return_type, parameters):
        return

    def processFileHeader(self, name):
        GeneratorBackend.processFileHeader(self, name)
        ts = (
        "#ifdef HAVE_CONFIG_H",
        "#include \"config.h\"",
        "#endif",
        self.importClassInclude(name, self.overrides.internalNamespace()),
        "",
        "$COMMON_INCLUDES_PRIVATE",
        "$CLASS_INCLUDES")
        self.puts('\n'.join(ts))
        self.EOL()

    def processNamespaceBegin(self, name):
        GeneratorBackend.processNamespaceBegin(self, name)
        self.puts("namespace %1 {\n", name)

    def processNamespaceEnd(self, name):
        GeneratorBackend.processNamespaceEnd(self, name)
        self.puts("} // namespace %1\n", name)

    def processClassBegin(self, name, native_export_macro, extends, class_type_parameters, has_trivial_constructor, has_native_constructors):
        GeneratorBackend.processClassBegin(self, name, native_export_macro, extends, class_type_parameters, has_trivial_constructor, has_native_constructors)
        ts = (
        "namespace $INTERNAL_NAMESPACE {",
        "",
        "static $LOCAL_REF<Natives::%1> nativeObject(JNI::ref_t thisObject);",
        "")
        self.puts('\n'.join(ts), name)
        self.EOL()

    def processClassEnd(self, name):
        self.implementDefaultConstructor()

        ts = (
        "class $CLASS_PATH::NativeBindings {",
        "public:",
        "")
        self.puts('\n'.join(ts))
        self.INC()

        managed_object_type = self.overrides.managedObjectType();

        if self.class_attribute.has_native_constructors:
            ts = (
            "static $INTERNAL_TYPE nativeObjectFieldGet($MANAGED_OBJECT thisObject)",
            "{",
            "")
            ts = string.Template('\n'.join(ts)).safe_substitute({
                                                 'INTERNAL_TYPE' : self.resolveInternalType(self.class_attribute.native_object_field.base_type, 0),
                                                 'MANAGED_OBJECT' : managed_object_type,
                                                 })
            self.puts(ts)
            self.INC()
            self.implementFieldAccess(self.class_attribute.native_object_field.name, False, 'thisObject', self.class_attribute.native_object_field.base_type, 0, 'get')
            self.DEC()
            self.puts("}")
            self.EOL()

        ts = (
        "static $LOCAL_REF<Natives::$CLASS_PATH> nativeObjectRef(%1 thisObject)",
        "{")
        self.puts('\n'.join(ts), managed_object_type)

        if self.class_attribute.has_native_constructors:
            ts = ("",
            "    JNI::NativeObject* nativeObject = reinterpret_cast<JNI::NativeObject*>(nativeObjectFieldGet(thisObject));",
            "    $ASSERT(nativeObject);",
            "    nativeObject->ref();")
        else:
            ts = ("",
            "    JNI::pushLocalCallerObjectRef(thisObject);",
            "    JNI::NativeObject* nativeObject = Natives::$CLASS_PATH::CTOR();")
        self.puts('\n'.join(ts))

        ts = ("",
        "    Natives::$CLASS_PATH* nativeThis = static_cast<Natives::$CLASS_PATH*>(nativeObject);",
        "    return JNI::adoptRef(thisObject, nativeThis);",
        "}")
        self.puts('\n'.join(ts))
        self.EOL()

        for native_constructor in self.class_attribute.native_constructors:
            self.implementNativeConstructor(native_constructor.name, native_constructor.parameters)
        if self.class_attribute.native_destructor is not None:
            self.implementNativeDestructor(self.class_attribute.native_destructor.name)
        for native in self.pending_native_methods:
            self.implementNativeBinding(native.is_static, native.is_abstract, native.return_type, native.name, native.parameters)

        self.DEC()
        self.puts("};\n")
        self.EOL()

        ts = (
        "static $LOCAL_REF<Natives::$CLASS_PATH> nativeObject(JNI::ref_t thisObject)",
        "{",
        "    return $CLASS_PATH::NativeBindings::nativeObjectRef(reinterpret_cast<%1>(thisObject));",
        "}",
        "")
        self.puts('\n'.join(ts), managed_object_type)
        self.EOL()

        self.implementRegisterClass()
        self.puts("} // namespace $INTERNAL_NAMESPACE")
        self.EOL()

        if self.class_attribute.has_native_constructors:
            self.template = self.template.replace("$NATIVE_OBJECT_FIELD", self.class_attribute.native_object_field.name)

        GeneratorBackend.processClassEnd(self, name)

    def processConstructor(self, called_by_native, parameters):
        GeneratorBackend.processConstructor(self, called_by_native, parameters)
        if called_by_native is False:
            return

        ts = (
        "$PASS_CLASS $CLASS_PATH::create(%1)",
        "{",
        "")
        self.puts('\n'.join(ts), self.buildParameters(parameters, True))
        self.INC()
        self.implementConstruction(called_by_native, parameters)
        self.DEC()
        self.puts("}\n")
        self.EOL()

    def processNativeObjectField(self, name, base_type):
        GeneratorBackend.processNativeObjectField(self, name, base_type)
        self.processField(True, False, False, base_type, 0, None, name)

    def processMethod(self, called_by_native, is_static, is_abstract, return_type, name, parameters):
        GeneratorBackend.processMethod(self, called_by_native, is_static, is_abstract, return_type, name, parameters)
        if called_by_native is False:
            return

        ts = (
        "$METHOD_RETURN_TYPE $CLASS_PATH::$METHOD_NAME($METHOD_PARAMETERS)",
        "{",
        "")
        ts = string.Template('\n'.join(ts)).safe_substitute({
                                             'METHOD_RETURN_TYPE' : self.buildMethodReturn(return_type),
                                             'METHOD_NAME' : name,
                                             'METHOD_PARAMETERS' : self.buildParameters(parameters, True),
                                             })
        self.puts(ts)
        self.INC()
        self.implementMethodInvocation(name, is_static, return_type, parameters)
        self.DEC()
        self.puts("}\n")
        self.EOL()

    def processNativeMethod(self, is_static, is_abstract, return_type, name, parameters):
        GeneratorBackend.processNativeMethod(self, is_static, is_abstract, return_type, name, parameters)
        assert(is_static or self.class_attribute.has_native_constructors)
        self.pending_native_methods.append(NativeMethod(name, parameters, is_static, is_abstract, return_type))

    def processField(self, accessed_by_native, is_static, is_final, base_type, dimensions, initializer, name):
        GeneratorBackend.processField(self, accessed_by_native, is_static, is_final, base_type, dimensions, initializer, name)
        if not accessed_by_native:
            return

        if is_final and initializer is not None:
            if isPrimitiveType(getTypeName(base_type)):
                return
            else:
                self.puts("const %2 $CLASS_PATH::%1", name, self.resolveInternalType(getTypeName(base_type), getTypeDimensions(base_type)))
                if initializer is not None:
                    self.puts(" = %1", getInitializerValue(initializer, base_type))
                self.puts(";\n")
                self.EOL()
                return

        if is_static:
            self.puts("$CLASS_PATH::FIELD_INTERFACE_CLASS_NAME(%1) $CLASS_PATH::%1;\n", name)
            self.EOL()

        ts = (
        "void $CLASS_PATH::FIELD_INTERFACE_CLASS_NAME(%1)::set(%2)",
        "{",
        "")
        self.puts('\n'.join(ts), name, self.buildParameter(Parameter(False, base_type, dimensions, "value"), True))
        self.INC()
        self.implementFieldAccess(name, is_static, None, base_type, dimensions, 'set')
        self.DEC()
        self.puts('}\n')
        self.EOL()

        ts = (
        "%2 $CLASS_PATH::FIELD_INTERFACE_CLASS_NAME(%1)::get()",
        "{",
        "")
        self.puts('\n'.join(ts), name, self.buildParameter(Parameter(False, base_type, dimensions, ""), False))
        self.INC()
        self.implementFieldAccess(name, is_static, None, base_type, dimensions, 'get')
        self.DEC()
        self.puts('}\n')
        self.EOL()

    def processEOF(self):
        GeneratorBackend.processEOF(self)
        header_includes = []
        if len(self.unknown_parameter_types) > 0:
            header_includes.append('')
            for unknown_type in self.unknown_parameter_types:
                header_includes.append(self.importClassInclude(unknown_type, self.overrides.internalNamespace()))
                if self.overrides.hasManaged():
                    header_includes.append(self.importClassInclude(unknown_type, self.overrides.externalNamespace()))
            header_includes.append('')
        self.template = self.template.replace("$CLASS_INCLUDES", '\n'.join(header_includes))

class NativesCPPStubGeneratorBackendOverrides(NativesHeaderGeneratorBackendOverrides):
    def __init__(self):
        NativesHeaderGeneratorBackendOverrides.__init__(self)

    def hasManaged(self):
        return True

    def managedObjectType(self):
        return 'JNI::ObjectReference*'

class NativesCPPStubGeneratorBackend(StubGeneratorBackend):
    def __init__(self):
        StubGeneratorBackend.__init__(self, NativesCPPStubGeneratorBackendOverrides())

    def implementDefaultConstructor(self):
        self.puts("$CLASS_PATH::$CLASS_PATH()\n")
        initializers = ""
        if len(self.native_fields) > 0:
            counter = 0
            for native_field in self.native_fields:
                initializers += ": " if counter == 0 else ", "
                initializers += native_field.name + "(NativeObject::m_bind)\n"
                counter += 1
        self.INC()
        self.puts(initializers)
        self.DEC()
        self.puts("{\n")
        self.puts("}\n")
        self.EOL()

    def implementRegisterClass(self):
        ts = (
        "// NOTE: SHOULD BE CALLED DURING MODULE INITIALIZATION",
        "bool $CLASS_PATH::registerClass()",
        "{",
        "    return true;",
        "}",
        "")
        self.puts('\n'.join(ts))
        self.EOL()

    def implementFieldAccess(self, field_name, is_static, this_reference, base_type, dimensions, get_or_set):
        ts = ("$SCOPE$ACCESSING_OPERATOR")
        if is_static:
            ts = string.Template(''.join(ts)).safe_substitute({
                                                 'SCOPE' : ''.join([managed_files_suffix, '::', self.classPath()]),
                                                 'ACCESSING_OPERATOR' : '::',
                                                 })
        else:
            scope = this_reference if this_reference is not None else "m_ref"
            self.puts("%1::$CLASS_PATH* managedThis = JNI::getPtr<%1::$CLASS_PATH>(%2);\n", managed_files_suffix, scope)
            ts = string.Template(''.join(ts)).safe_substitute({
                                                 'SCOPE' : 'managedThis',
                                                 'ACCESSING_OPERATOR' : '->',
                                                 })
        ts = ''.join([ts, field_name, ''.join([' = ', self.surroundWithCast(base_type, dimensions, 'value')]) if get_or_set == 'set' else ""])
        if get_or_set == 'get':
            ts = ''.join(['return ', self.surroundWithCast(base_type, dimensions, ts, False)])
        self.puts(''.join([ts, ';']))
        self.EOL()

    def implementConstruction(self, called_by_native, parameters):
        self.puts("return fromPtr($EXTERNAL_NAMESPACE::$CLASS_PATH::create(%1));\n", self.buildArguments(parameters, True))

    def implementMethodInvocation(self, function_name, is_static, return_type, parameters):
        has_result = return_type != 'void'

        ts = ("$SCOPE$ACCESSING_OPERATOR")
        if is_static:
            ts = string.Template(ts).safe_substitute({
                                                 'SCOPE' : ''.join([self.overrides.externalNamespace(), '::', self.classPath()]),
                                                 'ACCESSING_OPERATOR' : '::',
                                                 })
        else:
            self.puts("$EXTERNAL_NAMESPACE::$CLASS_PATH* managedThis = JNI::getPtr<$EXTERNAL_NAMESPACE::$CLASS_PATH>(NativeObject::m_bind);\n")
            ts = string.Template(ts).safe_substitute({
                                                 'SCOPE' : 'managedThis',
                                                 'ACCESSING_OPERATOR' : '->',
                                                 })
        ts = ''.join([ts, function_name, '(', ', '.join(self.buildArgumentList(parameters, True)), ')'])
        if has_result:
            ts = ''.join(['return ', self.surroundWithCast(getTypeName(return_type), getTypeDimensions(return_type), ts, False)])
        self.puts(''.join([ts, ';']))
        self.EOL()

    def processFileHeader(self, name):
        StubGeneratorBackend.processFileHeader(self, name)
        self.puts(self.importClassInclude(name, self.overrides.externalNamespace()))
        self.EOL()
        self.EOL()

    def processClassBegin(self, name, native_export_macro, extends, class_type_parameters, has_trivial_constructor, has_native_constructors):
        StubGeneratorBackend.processClassBegin(self, name, native_export_macro, extends, class_type_parameters, has_trivial_constructor, has_native_constructors)

        ts = (
        "$LOCAL_REF<$CLASS_PATH> $CLASS_PATH::fromRef(JNI::ref_t ref)",
        "{",
        "    return nativeObject(ref);",
        "}",
        "",
        "$LOCAL_REF<$CLASS_PATH> $CLASS_PATH::fromPtr(std::shared_ptr<$EXTERNAL_NAMESPACE::$CLASS_PATH>& ptr)",
        "{",
        "    return fromRef(reinterpret_cast<$CLASS_NAME*>(ptr->$NATIVE_OBJECT_FIELD)->refLocal());" if has_native_constructors else "    return fromRef(new JNI::ObjectReference(std::move(ptr)));",
        "}",
        "")
        self.puts('\n'.join(ts))
        self.EOL()

class NativesJNIStubGeneratorBackendOverrides(NativesCPPStubGeneratorBackendOverrides):
    def __init__(self):
        NativesCPPStubGeneratorBackendOverrides.__init__(self)

    def hasManaged(self):
        return False

    def managedObjectType(self):
        return 'jobject'

    def externalTypeOfAnyObject(self):
        return 'jobject'

    def externalTypeOfType(self, base_type, type_dimensions):
        return JNI_TYPE(base_type, type_dimensions)

    def externalTypeOfObject(self, object_type):
        return object_type

class NativesJNIStubGeneratorBackend(StubGeneratorBackend):
    def __init__(self):
        StubGeneratorBackend.__init__(self, NativesJNIStubGeneratorBackendOverrides())
        self.pending_native_methods = []
        self.native_method_registry = []

    def buildJNIClassID(self):
        return "ClassID_$CLASS_PATH()"

    def buildJNIParameters(self, parameters, indention=0):
        def buildJNIParameter(parameter):
            return self.resolveExternalType(parameter.base_type, parameter.dimensions) + ' ' + parameter.name

        result = ""
        tabs = tab_character * (self.indention + indention)
        if len(parameters) > 0:
            first_parameter = parameters[0]
            next_parameters = parameters[1:]
            result += buildJNIParameter(first_parameter)
            for parameter in next_parameters:
                result += '\n' + tabs + ', ' + buildJNIParameter(parameter)
        return result

    def buildJNIArguments(self, parameters, indention=0):
        def buildJNIArgument(parameter):
            to_object = ("<" + self.resolveInternalType(parameter.base_type, parameter.dimensions) + ">") if isObjectType(parameter.base_type) else ""
            return "JNI::toNative" + to_object + '(' + parameter.name + ')'

        result = ""
        tabs = tab_character * (self.indention + indention)
        if len(parameters) > 0:
            first_parameter = parameters[0]
            next_parameters = parameters[1:]
            result += buildJNIArgument(first_parameter)
            for parameter in next_parameters:
                result += '\n' + tabs + ', ' + buildJNIArgument(parameter)
        return result

    def buildJNISignature(self, base_type, dimensions):
        actual_type = self.resolveClassTypeParameter(base_type)
        return JNI_SIGNATURE(actual_type, dimensions)

    def buildJNISignatures(self, parameters, return_type):
        signature = "("
        for parameter in parameters:
            signature += self.buildJNISignature(parameter.base_type, parameter.dimensions)
        signature += ")"
        signature += self.buildJNISignature(getTypeName(return_type), getTypeDimensions(return_type))
        return signature

    def implementDefaultConstructor(self):
        self.puts("$CLASS_PATH::$CLASS_PATH()\n")
        initializers = ""
        if len(self.native_fields) > 0:
            counter = 0
            for native_field in self.native_fields:
                initializers += ": " if counter == 0 else ", "
                initializers += native_field.name + "(NativeObject::m_bind)\n"
                counter += 1
        self.INC()
        self.puts(initializers)
        self.DEC()
        self.puts("{\n")
        self.puts("}\n")
        self.EOL()

    def implementRegisterClass(self):
        ts = (
        "// NOTE: SHOULD BE CALLED DURING MODULE INITIALIZATION",
        "bool $CLASS_PATH::registerClass()",
        "{",
        "    %1;",
        "")
        self.puts('\n'.join(ts), self.buildJNIClassID())
        self.EOL()
        self.INC()

        if len(self.native_method_registry) > 0:
            native_methods = []
            for native_method in self.native_method_registry:
                native_methods.append(''.join(["{ \"", native_method.name, "\", \"", native_method.signatures, "\",\n"]))
                native_methods.append(''.join(["  (void*)$CLASS_NAME::NativeBindings::", native_method.function_name, " },\n"]))

            ts = (
            "static const JNINativeMethod k${CLASS_NAME}NativeMethods[] = {",
            "    %1"
            "};",
            "const int k${CLASS_NAME}NativeMethodsCount = sizeof(k${CLASS_NAME}NativeMethods) / sizeof(JNINativeMethod);",
            "if ($JNIENV->RegisterNatives(ClassID_${CLASS_NAME}(), k${CLASS_NAME}NativeMethods, k${CLASS_NAME}NativeMethodsCount) < 0) {",
            "    $LOG_ERROR(\"RegisterNatives failed for: ${CLASS_NAME}\");",
            "    return false;",
            "}",
            "")
            self.puts('\n'.join(ts), tab_character.join(native_methods))

        self.puts("return true;\n")
        self.DEC()
        self.puts("}\n")
        self.EOL()
        self.EOL()

    def implementFieldAccess(self, field_name, is_static, this_reference, base_type, dimensions, get_or_set):
        has_result = base_type != 'void' and get_or_set == 'get'

        ts = ("$RETURN$JNIENV->$CALL${CALL_TYPE}Field($SCOPE, FieldID_$FIELD_NAME()")
        jni_type = JNI_TYPE(base_type, dimensions)
        ts = string.Template(''.join(ts)).safe_substitute({
                                             'RETURN' : ''.join([jni_type, " result = (", jni_type, ')']) if has_result else "",
                                             'CALL' : ''.join(['Set' if get_or_set == 'set' else 'Get', 'Static' if is_static else '']),
                                             'CALL_TYPE' : CALL_TYPE(base_type, dimensions),
                                             'SCOPE' : this_reference if this_reference is not None else self.buildJNIClassID() if is_static else "reinterpret_cast<jobject>(m_ref)",
                                             'FIELD_NAME' : field_name
                                             })
        if get_or_set == 'set':
            ts = ''.join([ts, ', ', ''.join(self.surroundWithCast(base_type, dimensions, 'value', True))])
        self.puts(''.join([ts, ');']))
        self.EOL()
        if has_result:
            ts = ''.join(['return ', self.surroundWithCast(base_type, dimensions, 'result', False), ';'])
            self.puts(ts)
            self.EOL()

    def implementNativeConstructor(self, name, parameters):
        self.native_method_registry.append(NativeMethodRegistry(name, self.buildJNISignatures(parameters, 'void'), name))

        ts = (
        "static void %1(JNIEnv*, jobject scope$PRECEDING_COMMA$JNI_PARAMETERS)",
        "{",
        "    JNI::pushLocalCallerObjectRef(scope);",
        "    auto* nativePtr = $CLASS_PATH::%1($JNI_ARGUMENTS);",
        "    nativePtr->$NATIVE_OBJECT_FIELD.set(reinterpret_cast<jint>(nativePtr)); // Reference adopted",
        "}")
        ts = string.Template('\n'.join(ts)).safe_substitute({
                                             'PRECEDING_COMMA' : ', ' if len(parameters) > 0 else '',
                                             'JNI_PARAMETERS' : self.buildJNIParameters(parameters),
                                             'JNI_ARGUMENTS' : self.buildJNIArguments(parameters, 1),
                                             'NATIVE_OBJECT_FIELD' : self.class_attribute.native_object_field.name,
                                             })
        self.puts(ts, name)
        self.EOL()

    def implementNativeDestructor(self, name):
        assert(self.class_attribute.has_native_constructors)
        self.native_method_registry.append(NativeMethodRegistry(name, self.buildJNISignatures([], 'void'), name))

        ts = (
        "static void %1(JNIEnv*, jobject scope)",
        "{",
        "    reinterpret_cast<Natives::$CLASS_PATH*>(nativeObjectFieldGet(scope))->deref();",
        "}")
        self.puts('\n'.join(ts), name)
        self.EOL()

    def implementNativeBinding(self, is_static, is_abstract, return_type, name, parameters):
        self.native_method_registry.append(NativeMethodRegistry(name, self.buildJNISignatures(parameters, return_type), name))

        has_result = return_type != 'void'

        ts = ("static $JNI_TYPE $METHOD_NAME(JNIEnv*, $JNI_SCOPE$PRECEDING_COMMA$JNI_PARAMETERS)")
        ts = string.Template(''.join(ts)).safe_substitute({
                                             'JNI_TYPE' : self.resolveExternalType(getTypeName(return_type), getTypeDimensions(return_type)),
                                             'METHOD_NAME' : name,
                                             'JNI_SCOPE' : "jclass" if is_static else "jobject scope",
                                             'PRECEDING_COMMA' : ', ' if len(parameters) > 0 else '',
                                             'JNI_PARAMETERS' : self.buildJNIParameters(parameters),
                                             })
        self.puts(ts)
        self.EOL()

        ts = ("$SCOPE$ACCESSING_OPERATOR$METHOD_NAME($JNI_ARGUMENTS)")
        ts = string.Template(''.join(ts)).safe_substitute({
                                             'SCOPE' : self.classPath() if is_static else "nativeObject(scope)",
                                             'ACCESSING_OPERATOR' : '::' if is_static else '->',
                                             'METHOD_NAME' : name,
                                             'JNI_ARGUMENTS' : self.buildJNIArguments(parameters, 1),
                                             })
        if has_result:
            ts = ''.join(['return ', self.surroundWithCast(getTypeName(return_type), getTypeDimensions(return_type), ts, True)])
        ts = '\n'.join(['{', tab_character + ts + ';', '}'])
        self.puts(ts)
        self.EOL()

    def implementConstruction(self, called_by_native, parameters):
        ts = (
        "static jmethodID mid = $JNIENV->GetMethodID($CLASS_ID",
        "    , \"<init>\"",
        "    , \"$CONSTRUCTOR_SIGNATURE\");",
        "if (!mid) {",
        "    $LOG_ERROR(\"GetMethodID failed for: $CLASS_NAME.<init>\");",
        "}",
        "jobject result = $JNIENV->NewObject($CLASS_ID, mid$PRECEDING_COMMA$CONSTRUCTOR_ARGUMENTS);",
        "$ASSERT(result);",
        "return nativeObject(result);")
        ts = string.Template('\n'.join(ts)).safe_substitute({
                                             'CLASS_ID' : self.buildJNIClassID(),
                                             'CONSTRUCTOR_SIGNATURE' : self.buildJNISignatures(parameters, 'void'),
                                             'PRECEDING_COMMA' : ', ' if len(parameters) > 0 else '',
                                             'CONSTRUCTOR_ARGUMENTS' : self.buildArguments(parameters, True),
                                             })
        self.puts(ts)
        self.EOL()

    def implementMethodInvocation(self, function_name, is_static, return_type, parameters):
        has_result = return_type != 'void'

        ts = (
        "static jmethodID mid = $JNIENV->${GET}MethodID($CLASS_ID",
        "    , \"$METHOD_NAME\"",
        "    , \"$JNI_SIGNATURE\");",
        "if (!mid) {",
        "    $LOG_ERROR(\"${GET}MethodID failed for: $METHOD_NAME\");",
        "}")
        ts = string.Template('\n'.join(ts)).safe_substitute({
                                             'GET' : "GetStatic" if is_static else "Get",
                                             'CLASS_ID' : self.buildJNIClassID(),
                                             'METHOD_NAME' : function_name,
                                             'JNI_SIGNATURE' : self.buildJNISignatures(parameters, return_type),
                                             })
        self.puts(ts)
        self.EOL()

        ts = ("$RETURN$JNIENV->$CALL${CALL_TYPE}Method($SCOPE, mid")
        jni_type = JNI_TYPE(getTypeName(return_type), getTypeDimensions(return_type))
        ts = string.Template(''.join(ts)).safe_substitute({
                                             'RETURN' : ''.join([jni_type, " result = (", jni_type, ')']) if has_result else "",
                                             'CALL' : "CallStatic" if is_static else "Call",
                                             'CALL_TYPE' : CALL_TYPE(getTypeName(return_type), getTypeDimensions(return_type)),
                                             'SCOPE' : self.buildJNIClassID() if is_static else "reinterpret_cast<jobject>(NativeObject::m_bind)"
                                             })
        if len(parameters) > 0:
            ts = ''.join([ts, ', ', ', '.join(self.buildArgumentList(parameters, True))])
        self.puts(''.join([ts, ');']))
        self.EOL()
        if has_result:
            ts = ''.join(['return ', self.surroundWithCast(getTypeName(return_type), getTypeDimensions(return_type), 'result', False), ';'])
            self.puts(ts)
            self.EOL()

    def processFileHeader(self, name):
        StubGeneratorBackend.processFileHeader(self, name)
        self.puts("#define PACKAGE_NAME \"%1\"\n", self.nativePackageName())
        self.EOL()

    def processImport(self, name):
        StubGeneratorBackend.processImport(self, name)
        name_parts = string.split(name, '.')
        jni_signature_map.mapType(name_parts[-1], [''.join(['L', '/'.join(name_parts), ';'])])

    def processClassBegin(self, name, native_export_macro, extends, class_type_parameters, has_trivial_constructor, has_native_constructors):
        StubGeneratorBackend.processClassBegin(self, name, native_export_macro, extends, class_type_parameters, has_trivial_constructor, has_native_constructors)

        ts = (
        "$LOCAL_REF<$CLASS_PATH> $CLASS_PATH::fromRef(JNI::ref_t ref)",
        "{",
        "    return nativeObject(ref);",
        "}",
        "",
        "$LOCAL_REF<$CLASS_PATH> $CLASS_PATH::fromPtr(std::shared_ptr<$EXTERNAL_NAMESPACE::$CLASS_PATH>&)",
        "{",
        "    return $LOCAL_REF<$CLASS_PATH>(); // FIXME: Error if fromPtr() is used. This method should be removed.",
        "}",
        "",
        "static jclass ClassID_$CLASS_NAME()",
        "{",
        "    static jclass cid = reinterpret_cast<jclass>($JNIENV->NewGlobalRef($JNIENV->FindClass(PACKAGE_NAME \"/$CLASS_NAME\")));",
        "    return cid;",
        "}",
        "")
        self.puts('\n'.join(ts))
        self.EOL()

    def processField(self, accessed_by_native, is_static, is_final, base_type, dimensions, initializer, name):
        if not accessed_by_native:
            return

        if not is_final or initializer is None:
            ts = (
            "static jfieldID FieldID_$FIELD_NAME()",
            "{",
            "    static jfieldID fid = $JNIENV->${GET}FieldID($CLASS_ID",
            "        , \"$FIELD_NAME\"",
            "        , \"$FIELD_SIGNATURE\");",
            "    if (!fid) {",
            "        $LOG_ERROR(\"${GET}FieldID failed for: $FIELD_NAME\");",
            "    }",
            "    return fid;",
            "}",
            "")
            ts = string.Template('\n'.join(ts)).safe_substitute({
                                                 'FIELD_NAME' : name,
                                                 'GET' : "GetStatic" if is_static else "Get",
                                                 'CLASS_ID' : self.buildJNIClassID(),
                                                 'FIELD_SIGNATURE' : self.buildJNISignature(base_type, dimensions),
                                                 })
            self.puts(ts)
            self.EOL()

        StubGeneratorBackend.processField(self, accessed_by_native, is_static, is_final, base_type, dimensions, initializer, name)

class ManagedCPPStubGeneratorBackendOverrides(ManagedHeaderGeneratorBackendOverrides):
    def __init__(self):
        ManagedHeaderGeneratorBackendOverrides.__init__(self)

    def hasManaged(self):
        return True

    def managedObjectType(self):
        return 'JNI::ObjectReference*'

    def internalPassObject(self, object_type):
        return 'std::shared_ptr<$T>'.replace('$T', object_type)

class ManagedCPPStubGeneratorBackend(StubGeneratorBackend):
    def __init__(self):
        StubGeneratorBackend.__init__(self, ManagedCPPStubGeneratorBackendOverrides())

    def implementInitialization(self, constructor, initializer):
        ts = (
        "$PASS_CLASS uninitialized($CTOR);",
        "${REFERENCE_LOCAL}"
        "$INITIALIZE_LOCAL;",
        "${DEREFERENCE_LOCAL}"
        "return uninitialized;")
        ts = string.Template('\n'.join(ts)).safe_substitute({
                                             'CTOR' : constructor,
                                             'REFERENCE_LOCAL' : '\n'.join(["JNI::ref_t ref = new JNI::ObjectReference(uninitialized);",
                                                                            "JNI::pushLocalCallerObjectRef(ref);", ""])
                                                             if self.class_attribute.has_native_constructors else "",
                                             'INITIALIZE_LOCAL' : initializer,
                                             'DEREFERENCE_LOCAL' : '\n'.join(["assert(uninitialized->$NATIVE_OBJECT_FIELD);",
                                                                            "JNI::derefLocal(ref);", ""])
                                                             if self.class_attribute.has_native_constructors else "",
                                             })
        self.puts(ts)

    def implementDefaultConstructor(self):
        self.puts("$CLASS_PATH::$CLASS_PATH()\n")
        if len(self.native_fields) > 0:
            counter = 0
            self.INC()
            for native_field in self.native_fields:
                self.puts(": " if counter == 0 else ", ")
                self.puts("%1(%2)\n", native_field.name, str(getInitializerValue(native_field.initializer, native_field.base_type)))
                counter += 1
            self.DEC()
        self.puts("{\n")
        self.INC()
        self.DEC()
        self.puts("}\n")
        self.EOL()

    def implementFieldAccess(self, field_name, is_static, this_reference, base_type, dimensions, get_or_set):
        ts = ("$SCOPE$ACCESSING_OPERATOR")
        if is_static:
            ts = string.Template(''.join(ts)).safe_substitute({
                                                 'SCOPE' : ''.join([managed_files_suffix, '::', self.classPath()]),
                                                 'ACCESSING_OPERATOR' : '::',
                                                 })
        else:
            scope = this_reference if this_reference is not None else "m_ref"
            self.puts("%1::$CLASS_PATH* managedThis = JNI::getPtr<%1::$CLASS_PATH>(%2);\n", managed_files_suffix, scope)
            ts = string.Template(''.join(ts)).safe_substitute({
                                                 'SCOPE' : 'managedThis',
                                                 'ACCESSING_OPERATOR' : '->',
                                                 })
        ts = ''.join([ts, field_name, ''.join([' = ', self.surroundWithCast(base_type, dimensions, 'value')]) if get_or_set == 'set' else ""])
        if get_or_set == 'get':
            ts = ''.join(['return ', self.surroundWithCast(base_type, dimensions, ts, False)])
        self.puts(''.join([ts, ';']))
        self.EOL()

    def implementConstruction(self, called_by_native, parameters):
        self.implementInitialization("CTOR()", "uninitialized->INIT($ARGUMENTS)".replace('$ARGUMENTS', self.buildArguments(parameters, False)))
        self.EOL()

    def implementNativeConstructor(self, name, parameters):
        ts = (
        "void $CLASS_PATH::$CONSTRUCTOR_NAME($CONSTRUCTOR_PARAMETERS)",
        "{",
        "    auto* nativePtr = Natives::$CLASS_PATH::$CONSTRUCTOR_NAME($CONSTRUCTOR_ARGUMENTS);",
        "    $NATIVE_OBJECT_FIELD = reinterpret_cast<int32_t>(nativePtr); // Reference adopted",
        "}",
        "")
        ts = string.Template('\n'.join(ts)).safe_substitute({
                                             'CONSTRUCTOR_NAME' : name,
                                             'CONSTRUCTOR_PARAMETERS' : self.buildParameters(parameters, True),
                                             'CONSTRUCTOR_ARGUMENTS' : self.buildArguments(parameters, True),
                                             })
        self.puts(ts)
        self.EOL()

    def implementNativeDestructor(self, name):
        ts = (
        "void $CLASS_PATH::%1()",
        "{",
        "    reinterpret_cast<Natives::$CLASS_PATH*>($NATIVE_OBJECT_FIELD)->deref();",
        "}",
        "")
        self.puts('\n'.join(ts),  name)
        self.EOL()

    def processFileHeader(self, name):
        StubGeneratorBackend.processFileHeader(self, name)
        self.puts(self.importClassInclude(name, self.overrides.externalNamespace()))
        self.EOL()
        self.EOL()

    def processClassBegin(self, name, native_export_macro, extends, class_type_parameters, has_trivial_constructor, has_native_constructors):
        StubGeneratorBackend.processClassBegin(self, name, native_export_macro, extends, class_type_parameters, has_trivial_constructor, has_native_constructors)

        ts = (
        "static std::function<$CLASS_PATH* ()> s_factory;",
        "",
        "$CLASS_PATH* $CLASS_PATH::CTOR()",
        "{",
        "$NEW_CLASS",
        "    return s_factory();",
        "}",
        "",
        "void $CLASS_PATH::overrideCTOR(std::function<$CLASS_NAME* ()> factory)",
        "{",
        "    s_factory = factory;",
        "}",
        "")
        self.puts('\n'.join(ts))
        self.EOL()

        ts = (
        "$PASS_CLASS $CLASS_PATH::create(std::function<$CLASS_NAME* ()> constructor, std::function<void ($CLASS_NAME*)> initializer)",
        "{",
        "")
        self.puts('\n'.join(ts))
        self.INC()
        self.implementInitialization("constructor()", "initializer(uninitialized.get())")
        self.EOL()
        self.DEC()
        self.puts("}\n")
        self.EOL()

        if has_native_constructors:
            ts = (
            "static JNI::ref_t refLocal($CLASS_PATH* thisObject)",
            "{",
            "    return reinterpret_cast<$EXTERNAL_NAMESPACE::$CLASS_PATH*>(thisObject->$NATIVE_OBJECT_FIELD)->refLocal();",
            "}",
            "")
            self.puts('\n'.join(ts))
            self.EOL()

    def processClassEnd(self, name):
        if self.class_attribute.has_abstract_method:
            ts = ("    assert(s_factory);", "")
        else:
            ts = (
            "    if (!s_factory)",
            "        return new $CLASS_NAME();",
            "")
        self.template = self.template.replace("$NEW_CLASS", '\n'.join(ts))
        self.class_attribute.native_constructors = []
        self.class_attribute.native_destructor = None
        StubGeneratorBackend.processClassEnd(self, name)

    def processNativeConstructor(self, name, parameters, is_abstract):
        StubGeneratorBackend.processNativeConstructor(self, name, parameters, is_abstract)
        self.implementNativeConstructor(name, parameters)

    def processNativeDestructor(self, name, parameters, is_abstract):
        StubGeneratorBackend.processNativeDestructor(self, name, parameters, is_abstract)
        self.implementNativeDestructor(name)

    def processMethod(self, called_by_native, is_static, is_abstract, return_type, name, parameters):
        GeneratorBackend.processMethod(self, called_by_native, is_static, is_abstract, return_type, name, parameters)

    def processNativeMethod(self, is_static, is_abstract, return_type, name, parameters):
        StubGeneratorBackend.processNativeMethod(self, is_static, is_abstract, return_type, name, parameters)
        assert(is_static or self.class_attribute.has_native_constructors)

        has_result = getTypeName(return_type) != 'void'

        ts = ("$SCOPE$ACCESSING_OPERATOR")
        if is_static:
            ts = string.Template(ts).safe_substitute({
                                                 'SCOPE' : ''.join([self.overrides.externalNamespace(), '::$CLASS_PATH']),
                                                 'ACCESSING_OPERATOR' : '::',
                                                 })
        else:
            ts = string.Template(ts).safe_substitute({
                                                 'SCOPE' : 'nativeObject(refLocal(this))',
                                                 'ACCESSING_OPERATOR' : '->',
                                                 })
        ts = ''.join([ts, name, '(', ', '.join(self.buildArgumentList(parameters, True)), ')'])
        if has_result:
            ts = ''.join(['return ', self.surroundWithCast(getTypeName(return_type), getTypeDimensions(return_type), ts, False)])
        method_invocation = ts

        ts = (
        "$METHOD_RETURN_TYPE $CLASS_PATH::$METHOD_NAME($METHOD_PARAMETERS)",
        "{",
        "    $METHOD_INVOCATION;",
        "}",
        "")
        ts = string.Template('\n'.join(ts)).safe_substitute({
                                             'METHOD_NAME' : name,
                                             'METHOD_RETURN_TYPE' : self.buildMethodReturn(return_type),
                                             'METHOD_PARAMETERS' : self.buildParameters(parameters, True),
                                             'METHOD_INVOCATION' : method_invocation,
                                             })
        self.puts(ts)
        self.EOL()

    def processField(self, accessed_by_native, is_static, is_final, base_type, dimensions, initializer, name):
        GeneratorBackend.processField(self, accessed_by_native, is_static, is_final, base_type, dimensions, initializer, name)
        if not accessed_by_native or not is_static:
            return

        if is_final and initializer is not None:
            if isPrimitiveType(getTypeName(base_type)):
                return
            else:
                self.puts("const ")

        self.puts("%2 $CLASS_PATH::%1", name, self.resolveInternalType(getTypeName(base_type), getTypeDimensions(base_type)))
        if initializer is not None:
            self.puts(" = %1", getInitializerValue(initializer, base_type))
        self.puts(";\n")
        self.EOL()

keywords_for_natives = {
    'COMMON_INCLUDES' : "#include <androidjni/JNIIncludes.h>",
    'COMMON_INCLUDES_PRIVATE' : "#include <androidjni/MarshalingHelpers.h>",
    'CPPSTRING' : 'std::string',
    'ASSERT' : 'assert',
    'LOG_ERROR' : 'ALOGE',
    'JNIENV' : 'JNI::getEnv()',
    'AUTO_PTR' : 'std::unique_ptr',
    'LOCAL_REF' : 'JNI::PassLocalRef',
}

output_to_file = True

def generateBindings(frontend, backend, source_file, target_file):
    if source_file.isModifiedSince(target_file):
        frontend.generateFromSourceFile(source_file, backend)
        source = backend.substitute(keywords_for_natives)
        if output_to_file:
            target_file_handle = open(target_file, 'w')
            target_file_handle.write(source)
        else:
            print(source)

def generatedHeaderLocation(target_path, source_file, is_managed):
    target_header_path = ''.join([target_path, source_file.package])
    if not os.path.exists(target_header_path):
        os.makedirs(target_header_path)
    return ''.join([target_header_path, '/', source_file.filename_only, managed_files_suffix if is_managed else natives_files_suffix, "Base.h"])

def generateBindingsHeader(source_file, output_path):
    frontend = GeneratorFrontend()

    generateBindings(frontend, NativesHeaderGeneratorBackend(), source_file, generatedHeaderLocation(output_path, source_file, False))

def generateBindingsForJNI(source_file, output_path):
    frontend = GeneratorFrontend()

    target_file = ''.join([output_path, source_file.filename_only, natives_files_suffix, "Stub.cpp"])
    generateBindings(frontend, NativesJNIStubGeneratorBackend(), source_file, target_file)

def generateBindingsForCPP(source_file, output_path):
    frontend = GeneratorFrontend()

    generateBindings(frontend, ManagedHeaderGeneratorBackend(), source_file, generatedHeaderLocation(output_path, source_file, True))

    target_file = ''.join([output_path, source_file.filename_only, natives_files_suffix, "Stub.cpp"])
    generateBindings(frontend, NativesCPPStubGeneratorBackend(), source_file, target_file)

    target_file = ''.join([output_path, source_file.filename_only, managed_files_suffix, "Stub.cpp"])
    generateBindings(frontend, ManagedCPPStubGeneratorBackend(), source_file, target_file)

if __name__ == '__main__':
    argparser = argparse.ArgumentParser()
    argparser.add_argument('--java', type=str, nargs='+', help='.java file(s) which interfaces will be generated from')
    argparser.add_argument('--shared', type=str, help='Path to put generated shared headers')
    argparser.add_argument('--android', type=str, help='Path to put generated Android C++ bindings')
    argparser.add_argument('--generic', type=str, help='Path to put generated generic C++ bindings')
    argparser.add_argument('--force', action='store_true', help='Forces interface generation')
    if len(sys.argv) <= 1:
        argparser.print_usage()
        sys.exit(1)
    else:
        args = argparser.parse_args()

    def normalizedFilePath(filepath):
        if sys.platform == 'cygwin':
            filepath = string.replace(filepath, '\\', '/')
        return filepath

    def normalizedDirectoryPath(filepath):
        filepath = normalizedFilePath(filepath)
        if filepath[-1] != os.sep:
            filepath += os.sep
        try:
            os.makedirs(filepath)
        except OSError as exception:
            if exception.errno != errno.EEXIST:
                raise
        return filepath

    for java in args.java:
        source_file = SourceFile(normalizedFilePath(java), args.force)

        if args.shared is not None:
            generateBindingsHeader(source_file, normalizedDirectoryPath(args.shared))

        if args.android is not None:
            generateBindingsForJNI(source_file, normalizedDirectoryPath(args.android))

        if args.generic is not None:
            generateBindingsForCPP(source_file, normalizedDirectoryPath(args.generic))
