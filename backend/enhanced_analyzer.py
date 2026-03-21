import re
import uuid
from enum import Enum
from typing import List, Dict, Any, Optional, Set
##
class Severity(Enum):
    ERROR = "error"
    WARNING = "warning"
    INFO = "info"
    SUGGESTION = "suggestion"

class Problem:
    def __init__(self, line: int, column: int, message: str, severity: Severity, error_id: str, 
                 scope: str = "global", symbol: str = None, code_snippet: str = None, 
                 suggestion: str = None, quick_fixes: List[Dict] = None):
        self.id = str(uuid.uuid4())
        self.line = line
        self.column = column
        self.message = message
        self.severity = severity.value
        self.error_id = error_id
        self.scope = scope
        self.symbol = symbol
        self.code_snippet = code_snippet
        self.suggestion = suggestion
        self.quick_fixes = quick_fixes or []

    def to_dict(self):
        return {
            "id": self.id,
            "line": self.line,
            "column": self.column,
            "message": self.message,
            "severity": self.severity,
            "error_id": self.error_id,
            "scope": self.scope,
            "symbol": self.symbol,
            "code": self.code_snippet,
            "suggestion": self.suggestion,
            "quickFix": self.quick_fixes
        }

class EnhancedAnalyzer:
    def __init__(self):
        self.reserved_keywords = {
            "int", "float", "char", "double", "void", "long", "short", "bool",
            "if", "else", "for", "while", "do", "switch", "case", "default",
            "break", "continue", "return", "struct", "typedef", "extern",
            "static", "register", "auto", "const", "volatile", "unsigned",
            "signed", "goto", "union", "enum", "sizeof"
        }
        self.valid_types = {"int", "float", "char", "double", "void", "bool", "long", "short"}

    def analyze(self, code: str) -> List[Dict[str, Any]]:
        problems = []
        if not code.strip():
            return []

        lines = code.split('\n')
        
        # State Tracking
        scope_stack = [{"name": "global", "symbols": {}, "level": 0}]
        all_symbols = {} # name -> {line, type, kind, used, initialized, scope}
        
        type_pattern = r'\b(' + '|'.join(self.valid_types) + r')\b'
        ident_pattern = r'\b([a-zA-Z_][a-zA-Z0-9_]*)\b'
        
        brace_level = 0
        open_braces_pos = [] # track lines of open braces
        parens_level = 0
        
        for i, line_content in enumerate(lines, 1):
            clean_line = line_content.split('//')[0].strip()
            full_line = line_content.split('//')[0]
            
            if not clean_line:
                continue

            # --- SYNTAX: Brackets/Parentheses Balance ---
            for idx, char in enumerate(full_line):
                if char == '{':
                    brace_level += 1
                    open_braces_pos.append(i)
                    scope_stack.append({"name": f"block_L{i}", "symbols": {}, "level": brace_level})
                elif char == '}':
                    if brace_level > 0:
                        brace_level -= 1
                        open_braces_pos.pop()
                        scope_stack.pop()
                    else:
                        problems.append(Problem(i, idx + 1, "Extra closing brace '}' found", Severity.ERROR, "SYNTAX_EXTRA_BRACE", 
                                               suggestion="Remove the extra '}' or ensure matching '{'").to_dict())
                elif char == '(':
                    parens_level += 1
                elif char == ')':
                    if parens_level > 0:
                        parens_level -= 1
                    else:
                        problems.append(Problem(i, idx + 1, "Mismatched parenthesis: extra ')'", Severity.ERROR, "SYNTAX_EXTRA_PAREN",
                                               suggestion="Check for a missing '('").to_dict())

            # --- IMMEDIATE: Incomplete Declaration Check ---
            # Check for "int main(" style incomplete lines
            func_decl_incomplete = re.search(type_pattern + r'\s+' + ident_pattern + r'\s*\(\s*$', clean_line)
            if func_decl_incomplete:
                problems.append(Problem(i, full_line.find('(') + 1, "Incomplete function declaration: expected ')' and body", Severity.ERROR, "SYNTAX_INCOMPLETE_FUNC",
                                       suggestion="Add closing ')' and function body '{ ... }'",
                                       code_snippet=clean_line).to_dict())

            # --- SYNTAX: Missing Semicolon ---
            # Basic check: if it's not a block start/end, or control structure, it usually needs a semicolon
            if clean_line and not any(clean_line.endswith(x) for x in ['{', '}', ';', ':', ',']) and not any(clean_line.startswith(x) for x in ['#', 'if', 'for', 'while', 'switch', 'else']):
                # Don't flag if it looks like a function header
                if '(' in clean_line and ')' not in clean_line:
                    pass # Handled by incomplete dec
                elif '(' in clean_line and ')' in clean_line and '{' not in clean_line:
                    problems.append(Problem(i, len(full_line), "Possible missing semicolon ';' or function body '{'", Severity.ERROR, "SYNTAX_MISSING_SEMI",
                                           suggestion="Add ';' at end of statement or '{' to start function body").to_dict())
                else:
                    problems.append(Problem(i, len(full_line), "Expected semicolon ';' at end of statement", Severity.ERROR, "SYNTAX_MISSING_SEMI",
                                           suggestion="Add ';' to terminate this statement").to_dict())

            # --- DECLARATION: Identifiers and Keywords ---
            # Detect Declarations
            # Matches: int x;   float y = 10;   void func(int a) {
            decl_match = re.search(type_pattern + r'\s+' + ident_pattern + r'(\s*[\(;,=])', clean_line)
            if decl_match:
                d_type = decl_match.group(1)
                name = decl_match.group(2)
                suffix = decl_match.group(3).strip()
                
                # Invalid Identifier (Starts with number) - Regex handles \b so it won't match invalid ones
                # But we can check for explicitly invalid ones
                invalid_ident = re.search(r'\b(\d+[a-zA-Z_]\w*)\b', full_line)
                if invalid_ident:
                    problems.append(Problem(i, invalid_ident.start(), f"Invalid identifier '{invalid_ident.group(1)}': Cannot start with a number", Severity.ERROR, "DECL_INVALID_NAME").to_dict())

                # Reserved Keywords
                if name in self.reserved_keywords:
                    problems.append(Problem(i, decl_match.start(2), f"'{name}' is a C reserved keyword", Severity.ERROR, "DECL_RESERVED",
                                           suggestion="Choose a different name like '" + name + "_'").to_dict())
                
                # Redeclaration / Shadowing
                is_redeclared = False
                current_scope = scope_stack[-1]
                if name in current_scope['symbols']:
                    prev_line = current_scope['symbols'][name]['line']
                    problems.append(Problem(i, decl_match.start(2), f"Redeclaration of '{name}' in this scope (previous at line {prev_line})", Severity.ERROR, "DECL_DUP",
                                           symbol=name).to_dict())
                    is_redeclared = True
                else:
                    # Shadowing check (parent scopes)
                    for scope in reversed(scope_stack[:-1]):
                        if name in scope['symbols']:
                             problems.append(Problem(i, decl_match.start(2), f"Variable '{name}' shadows a variable from parent scope '{scope['name']}'", Severity.WARNING, "SCOPE_SHADOWING",
                                                    symbol=name, suggestion="Rename to avoid confusion with parent scope").to_dict())
                             break
                    
                if not is_redeclared:
                    current_scope['symbols'][name] = {"line": i, "type": d_type}
                    all_symbols[name] = {
                        "line": i, "type": d_type, "used": False, 
                        "initialized": "=" in suffix or "(" in suffix,
                        "kind": "function" if "(" in suffix else "variable",
                        "scope": current_scope['name']
                    }

            # --- TYPE: Assignments ---
            if "=" in clean_line and not decl_match:
                # Basic assignment type check: int x = "string";
                assign_match = re.search(r'\b([a-zA-Z_]\w*)\s*=\s*(.*);', clean_line)
                if assign_match:
                    var_name = assign_match.group(1)
                    val = assign_match.group(2).strip()
                    
                    if var_name in all_symbols:
                        all_symbols[var_name]['initialized'] = True
                        v_type = all_symbols[var_name]['type']
                        # Simple type compatibility check
                        if v_type == 'int' and (val.startswith('"') or val.startswith("'")):
                             problems.append(Problem(i, assign_match.start(2), f"Type mismatch: cannot assign string/char literal to int variable '{var_name}'", Severity.ERROR, "TYPE_MISMATCH",
                                                    symbol=var_name).to_dict())
                        elif v_type == 'char' and not (val.startswith("'") or val.isdigit()):
                             problems.append(Problem(i, assign_match.start(2), f"Type mismatch: '{var_name}' is a char but assigned a non-char value", Severity.WARNING, "TYPE_COERCION").to_dict())

            # --- INITIALIZATION: Usage Tracking ---
            potential_usages = re.findall(ident_pattern, clean_line)
            for usage in potential_usages:
                if usage not in self.reserved_keywords and usage not in self.valid_types:
                    # Skip if it's the identifier being declared on this line
                    if decl_match and usage == decl_match.group(2):
                        continue
                    
                    # Mark as used if declared anywhere
                    found_decl = False
                    for scope in reversed(scope_stack):
                        if usage in scope['symbols']:
                            found_decl = True
                            break
                    
                    if found_decl:
                        if usage in all_symbols:
                            all_symbols[usage]['used'] = True
                            if not all_symbols[usage]['initialized'] and all_symbols[usage]['kind'] == 'variable':
                                # Warning: Used before initialization
                                problems.append(Problem(i, 0, f"Variable '{usage}' might be uninitialized when used here", Severity.WARNING, "INIT_UNINITIALIZED",
                                                       symbol=usage, suggestion=f"Initialize '{usage}' (e.g., int {usage} = 0;)").to_dict())

        # --- FINAL: Scope & Unused Checks ---
        # Unexpected EOF (unclosed braces)
        if brace_level > 0:
            last_open = open_braces_pos[-1]
            problems.append(Problem(len(lines), 0, f"Unexpected EOF: Unclosed block '{scope_stack[-1]['name']}' (opened at line {last_open})", Severity.ERROR, "SYNTAX_UNCLOSED_BRACE",
                                   suggestion="Add missing '}' at the end of the file").to_dict())
        
        if parens_level > 0:
            problems.append(Problem(len(lines), 0, "Mismatched parentheses: expected ')'", Severity.ERROR, "SYNTAX_UNCLOSED_PAREN").to_dict())

        # Unused Symbols
        for name, info in all_symbols.items():
            if not info['used'] and info['kind'] == 'variable' and name != 'main':
                problems.append(Problem(info['line'], 0, f"Local variable '{name}' is unused", Severity.INFO, "REF_UNUSED",
                                       symbol=name, suggestion="Remove or use the declared variable").to_dict())

        return problems
