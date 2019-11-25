from clang.cindex import TranslationUnit, Cursor, CursorKind
from ipc_data import Field, Method, Param, Struct
import re

def normalized_type(name, status):
    name = re.sub(r'^.*\(', '(', name)
    is_const = bool(re.match(r'^(?:const .*|.* [&*]const)$', name))
    is_ptr = bool(re.match(r'^.* \*(?:const)?$', name))
    is_ref = bool(re.match(r'^.* &(?:const)?$', name))
    if is_const:
        name = re.sub(r'^const ', '', name)
        name = re.sub(r' \*const$', '', name)
    if is_ptr:
        name = re.sub(r' \*(?:const)?$', '', name)
    if is_ref:
        name = re.sub(r' &(?:const)?$', '', name)
    return status['index'].get(name, name), is_const, is_ptr, is_ref

def get_metadata(node: Cursor, lines: List[str]):
    return {}

def empty_status(): return { 'index': {}, 'parsed': [] }

def parse_struct(node: Cursor, lines: List[str], status = empty_status()):
    name, _, _, _ = normalized_type(node.canonical.type.spelling, status)
    metadata = get_metadata(node, lines)
    cs = parse_children(node, lines, { **status, 'parsed': [] })
    fields = [f for f in cs['parsed'] if isinstance(f, Field)]
    methods = [m for m in cs['parsed'] if isinstance(m, Method)]
    s = Struct(name, metadata, fields, methods)
    return {
        **status,
        'index': { **status['index'], **cs['index'], name: s }
    }

def parse_children(node: Cursor, lines: List[str], status = empty_status()):
    for c in node.get_children():
        status = parse_node(c, lines, status)
    return status

def parse_method(node: Cursor, lines: List[str], status = empty_status()):
    name = node.canonical.displayname
    last_parenthesis_index = name.rfind('(') # it could be an operator
    name = name[:last_parenthesis_index]
    metadata = get_metadata(node, lines)
    result_type = node.canonical.result_type.spelling
    cs = parse_children(node, lines, { **status, 'parsed': [] })
    params = [ p for p in cs['parsed'] if isinstance(p, Param) ]
    m = Method(name, metadata, params, result_type)
    return {
        **status,
        'index': { **status['index'], **cs['index'] },
        'parsed': status['parsed'] + [m]
    }

def parse_field(node: Cursor, lines: List[str], status = empty_status()):
    name = node.canonical.displayname
    metadata = get_metadata(node, lines)
    fieldType, _, _, _ = normalized_type(node.canonical.type.spelling, status)
    f = Field(name, metadata, fieldType)
    return { **status, 'parsed': status['parsed'] + [f] }

def parse_param(node: Cursor, lines: List[str], status = empty_status()):
    typename, is_const, is_ptr, is_ref = normalized_type(node.canonical.type.spelling, status)

    return { **status, 'parsed': status['parsed'] + [Param(typename, is_const, is_ptr, is_ref)] }

def parse_node(node: Cursor, lines: List[str], status = empty_status()):
    file = node.location.file
    if file and re.match(r'^(?:/usr|/..)', file.name): return status
    return {
        CursorKind.STRUCT_DECL: parse_struct,
        CursorKind.CLASS_DECL: parse_struct,
        CursorKind.CXX_METHOD: parse_method,
        CursorKind.FIELD_DECL: parse_field,
        CursorKind.PARM_DECL: parse_param,
        CursorKind.NAMESPACE: parse_children,
        CursorKind.TRANSLATION_UNIT: parse_children,
    }.get(node.canonical.kind, lambda n, l, s: s)(node, lines, status)


def parse_file(file):

    # transform our file into an array of lines
    with open(file) as f:
        lines = [line for line in f]

        # creating our translation unit
        tu = TranslationUnit.from_source(file)

        return parse_node(tu.cursor, lines)
