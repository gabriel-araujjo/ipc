from clang.cindex import TranslationUnit, Cursor, CursorKind
from ipc_parser import parse_file



# --------------------------------------------------------------------------- #


class message:
    def __init__(self, facade, method):
        self.facade = facade
        self.method = method


def args(method):
    return method.args or []


def methods(struct):
    return struct.methods or []


def is_facade(struct):
    """
    Returns whether the struct name ends with the word `facade`.

    This function works with both snake_case and CamelCase patterns.
    """
    return re.match(r'.*(?:_f|F)acade$', struct.name) is not None


def is_listener(struct):
    """
    Returns whether the struct name either starts with `on` or ends with
    `listener`.

    This function works with both snake_case and CamelCase patterns.
    """
    matches = re.match(r'(?:^[oO]n[_A-Z].*|.*(?:_l|L)istener$)', struct.name)
    return matches is not None

# parse all structs
# find facades
# for each facade genrate_boilerplate(facade, method)

def generate_boilerplate(facade: Struct):


def main():
    file = sys.argv[1] if len(sys.argv) >= 2 else 'example/mouse_facade.hpp'
    parser_result = parse_file(file)
    structs = parse_file(file)['index'].values()
    facades = filter(is_facade, structs)
    messages = [message(f, m) for f in facades for m in methods(f)]
    print(message)


if __name__ == '__main__':
    main()
