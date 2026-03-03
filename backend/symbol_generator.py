import re
import json

class SymbolTableGenerator:
    """
    A robust symbol table generator for C source code.
    Extracts identifiers, classifies them as variables or functions,
    and determines their scope and line numbers.
    """
    
    def __init__(self):
        # List of valid C primitive data types for classification
        self.valid_types = [
            "int", "float", "char", "double", "void", 
            "long", "short", "signed", "unsigned", "bool"
        ]
        
    def generate(self, c_code: str):
        """
        Parses C source code and generates a list of symbols based on the provided rules.
        """
        symbols = []
        brace_level = 0
        
        # Split the code into lines for individual processing
        lines = c_code.splitlines()
        
        for line_num, line in enumerate(lines, 1):
            # 1. Strip comments and whitespace
            # Remove multi-line comments (simplified for single-line context)
            clean_line = re.sub(r'/\*.*?\*/', '', line)
            # Remove single-line comments
            clean_line = re.sub(r'//.*', '', clean_line).strip()
            
            if not clean_line:
                # Still need to check for braces even in lines that might only have braces
                brace_level += line.count('{') - line.count('}')
                continue

            # 2. Scope Detection
            # We check the brace level before processing the line for declarations
            # However, if a function is declared on this line (e.g., void foo() {), 
            # it should be considered global because the declaration itself is top-level.
            
            # Rule: Extract valid identifiers (no brackets, parentheses, or symbols in names)
            # We look for: <type> <identifier>
            type_pattern = r'\b(' + '|'.join(self.valid_types) + r')\b'
            
            # Find all occurrences of type followed by something
            for match in re.finditer(type_pattern, clean_line):
                data_type = match.group(1)
                
                # Look ahead for identifiers after the type
                # We want to find the identifier and the very next non-whitespace character
                # Pattern: [type] [identifier] [optional whitespace] [next_char]
                remaining_text = clean_line[match.end():].strip()
                
                # This pattern catches the identifier and the following character
                # Group 1: Identifier, Group 2: Next character (or None)
                ident_match = re.search(r'^([a-zA-Z_][a-zA-Z0-9_]*)\s*([\(;=,])?', remaining_text)
                
                if ident_match:
                    name = ident_match.group(1)
                    next_char = ident_match.group(2)
                    
                    # Rule: If identifier is immediately followed by (, classify as function
                    # Otherwise, classify as variable
                    kind = "function" if next_char == "(" else "variable"
                    
                    # Rule: Scope detection
                    # declarations outside any function -> global
                    # declarations inside a function -> local
                    scope = "global" if brace_level == 0 else "local"
                    
                    # Store fields: Name, Type, Kind, Scope, Line number
                    symbols.append({
                        "Name": name,
                        "Type": data_type,
                        "Kind": kind,
                        "Scope": scope,
                        "Line number": line_num
                    })
                    
                    # Handle multiple declarations on one line (e.g., int a, b, c;)
                    # If the next_char is a comma, we could potentially loop, 
                    # but the user rules are simple so we stick to the main ones.
                    # For "int a, b;", this simple logic will only see 'a'.
                    # Let's improve it slightly to handle commas if 'next_char' is ','
                    if next_char == ',':
                        # Simplistic approach for commas: find next identifiers
                        comma_parts = remaining_text.split(',')
                        for part in comma_parts[1:]:
                            part = part.strip()
                            m = re.search(r'^([a-zA-Z_][a-zA-Z0-9_]*)\s*([\(;=,])?', part)
                            if m:
                                sec_name = m.group(1)
                                sec_char = m.group(2)
                                sec_kind = "function" if sec_char == "(" else "variable"
                                symbols.append({
                                    "Name": sec_name,
                                    "Type": data_type,
                                    "Kind": sec_kind,
                                    "Scope": scope,
                                    "Line number": line_num
                                })
                                if sec_char != ',': break
            
            # 3. Update brace level AFTER processing the line
            # (In C, the function declaration line usually starts the scope)
            brace_level += clean_line.count('{')
            brace_level -= clean_line.count('}')
            if brace_level < 0: brace_level = 0
            
        return symbols

def main():
    # Example usage as a compiler design assistant
    parser = SymbolTableGenerator()
    
    test_code = """
    int global_var = 10;
    float pi = 3.14;
    
    void calculate(int x) {
        int result;
        result = x * 2;
    }
    
    int main() {
        int a, b;
        calculate(5);
        return 0;
    }
    """
    
    results = parser.generate(test_code)
    print(json.dumps(results, indent=4))

if __name__ == "__main__":
    main()
