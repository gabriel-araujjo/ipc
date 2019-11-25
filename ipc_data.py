from typing import List

class Element:
    def __init__(self, name, metadata):
        self.name = name
        self.metadata = metadata

class Field(Element):
    def __init__(self, name: str, metadata, type):
        super().__init__(name, metadata)
        self.type = type

class Param:
    def __init__(self, type, is_const, is_ptr, is_ref):
        self.type = type
        self.is_const = is_const
        self.is_ptr = is_ptr
        self.is_ref = is_ref

class Method(Element):
    def __init__(self, name: str, metadata, params: List[Param], result):
        super().__init__(name, metadata)
        self.params = params
        self.result = result

class Struct(Element):
    def __init__(self, name: str, metadata, fields: List[Field], methods: List[Method]):
        super().__init__(name, metadata)
        self.fields = fields
        self.methods = methods
