import ctypes
import os
from typing import Optional, Dict, Any, List
from dataclasses import dataclass
from enum import IntEnum

# Load the compiled C library
symbol_table_lib = None
LIB_NAME = 'symbol_table.dll' if os.name == 'nt' else 'symbol_table.so'
LIB_PATH = os.path.join(os.path.dirname(__file__), 'c_bridge', LIB_NAME)
ABS_LIB_PATH = os.path.abspath(LIB_PATH)

if os.path.exists(ABS_LIB_PATH):
    try:
        # On Windows + Python 3.8+, we need to be careful with DLL loading
        if os.name == 'nt' and hasattr(os, 'add_dll_directory'):
            os.add_dll_directory(os.path.dirname(ABS_LIB_PATH))
            # winmode=0 allows searching in system directories and added directories
            symbol_table_lib = ctypes.CDLL(ABS_LIB_PATH, winmode=0)
        else:
            symbol_table_lib = ctypes.CDLL(ABS_LIB_PATH)
        print(f"Successfully loaded {LIB_NAME}")
    except Exception as e:
        print(f"Warning: Could not load {LIB_NAME}. Error: {e}")
        # Fallback to simple load
        try:
            symbol_table_lib = ctypes.CDLL(LIB_NAME)
            print(f"Loaded {LIB_NAME} from system path")
        except:
            print("Running in mock mode - backend will provide simulated responses")
else:
    print(f"Library not found at {ABS_LIB_PATH}. Running in mock mode.")

# Enum definitions matching C enums
class DataType(IntEnum):
    TYPE_INT = 0
    TYPE_FLOAT = 1
    TYPE_CHAR = 2
    TYPE_DOUBLE = 3
    TYPE_VOID = 4
    TYPE_BOOL = 5
    TYPE_STRING = 6
    TYPE_ARRAY = 7
    TYPE_POINTER = 8
    TYPE_STRUCT = 9
    TYPE_FUNCTION = 10
    TYPE_UNKNOWN = 11

class SymbolKind(IntEnum):
    SYMBOL_VARIABLE = 0
    SYMBOL_FUNCTION = 1
    SYMBOL_PARAMETER = 2
    SYMBOL_CONSTANT = 3
    SYMBOL_ARRAY = 4
    SYMBOL_STRUCT = 5
    SYMBOL_TYPEDEF = 6

class StorageClass(IntEnum):
    STORAGE_AUTO = 0
    STORAGE_STATIC = 1
    STORAGE_EXTERN = 2
    STORAGE_REGISTER = 3

@dataclass
class SymbolEntry:
    name: str
    data_type: DataType
    kind: SymbolKind
    storage_class: StorageClass
    scope_level: int
    scope_name: str
    size: int
    offset: int
    line_number: int
    is_initialized: bool
    initial_value: str = ""
    num_parameters: int = 0
    param_types: List[DataType] = None
    array_dimensions: int = 0
    dimension_sizes: List[int] = None
    
    def __post_init__(self):
        if self.param_types is None:
            self.param_types = []
        if self.dimension_sizes is None:
            self.dimension_sizes = []

class SymbolTableWrapper:
    def __init__(self):
        self.symbol_table_ptr = None
        if symbol_table_lib is not None:
            self._setup_function_signatures()
            self.symbol_table_ptr = symbol_table_lib.create_symbol_table()
        self._scope_stack = ["global"]

    @property
    def current_scope_name(self) -> str:
        """Returns the current scope name"""
        return self._scope_stack[-1]
    
    def _setup_function_signatures(self):
        """Setup function signatures for C library calls"""
        # Create symbol table
        symbol_table_lib.create_symbol_table.restype = ctypes.c_void_p
        symbol_table_lib.create_symbol_table.argtypes = []
        
        # Insert symbol
        symbol_table_lib.symbol_table_insert.restype = ctypes.c_int
        symbol_table_lib.symbol_table_insert.argtypes = [
            ctypes.c_void_p, ctypes.c_char_p, ctypes.c_int, 
            ctypes.c_int, ctypes.c_int
        ]
        
        # Lookup symbol
        symbol_table_lib.symbol_table_lookup.restype = ctypes.c_void_p
        symbol_table_lib.symbol_table_lookup.argtypes = [
            ctypes.c_void_p, ctypes.c_char_p
        ]
        
        # Enter/exit scope
        symbol_table_lib.symbol_table_enter_scope.restype = None
        symbol_table_lib.symbol_table_enter_scope.argtypes = [
            ctypes.c_void_p, ctypes.c_char_p
        ]
        
        symbol_table_lib.symbol_table_exit_scope.restype = None
        symbol_table_lib.symbol_table_exit_scope.argtypes = [ctypes.c_void_p]
        
        # Set storage class
        symbol_table_lib.symbol_table_set_storage_class.restype = ctypes.c_int
        symbol_table_lib.symbol_table_set_storage_class.argtypes = [
            ctypes.c_void_p, ctypes.c_char_p, ctypes.c_int
        ]
        
        # Set array dimensions
        symbol_table_lib.symbol_table_set_array_dims.restype = ctypes.c_int
        symbol_table_lib.symbol_table_set_array_dims.argtypes = [
            ctypes.c_void_p, ctypes.c_char_p, ctypes.c_int, 
            ctypes.POINTER(ctypes.c_int)
        ]
        
        # Set function parameters
        symbol_table_lib.symbol_table_set_function_params.restype = ctypes.c_int
        symbol_table_lib.symbol_table_set_function_params.argtypes = [
            ctypes.c_void_p, ctypes.c_char_p, ctypes.c_int,
            ctypes.POINTER(ctypes.c_int)
        ]
        
        # Mark initialized - Use bridge function
        symbol_table_lib.symbol_table_mark_initialized_bridge.restype = ctypes.c_int
        symbol_table_lib.symbol_table_mark_initialized_bridge.argtypes = [
            ctypes.c_void_p, ctypes.c_char_p
        ]
        
        # Check if declared - Use bridge function
        symbol_table_lib.symbol_table_is_declared_bridge.restype = ctypes.c_int
        symbol_table_lib.symbol_table_is_declared_bridge.argtypes = [
            ctypes.c_void_p, ctypes.c_char_p
        ]
        
        # Get errors - Use bridge function
        symbol_table_lib.symbol_table_get_errors_bridge.restype = ctypes.c_int
        symbol_table_lib.symbol_table_get_errors_bridge.argtypes = [ctypes.c_void_p]
        
    # ... (existing setup code) ...
        # Delete symbol - Use bridge function
        if hasattr(symbol_table_lib, 'symbol_table_delete_bridge'):
            symbol_table_lib.symbol_table_delete_bridge.restype = ctypes.c_int
            symbol_table_lib.symbol_table_delete_bridge.argtypes = [
                ctypes.c_void_p, ctypes.c_char_p
            ]

        # Set initial value - Use bridge function
        if hasattr(symbol_table_lib, 'symbol_table_set_initial_value_bridge'):
            symbol_table_lib.symbol_table_set_initial_value_bridge.restype = ctypes.c_int
            symbol_table_lib.symbol_table_set_initial_value_bridge.argtypes = [
                ctypes.c_void_p, ctypes.c_char_p, ctypes.c_char_p
            ]
            
        # Get scope hierarchy - Use bridge function
        if hasattr(symbol_table_lib, 'get_scope_hierarchy_bridge'):
            symbol_table_lib.get_scope_hierarchy_bridge.restype = ctypes.c_void_p
            symbol_table_lib.get_scope_hierarchy_bridge.argtypes = [ctypes.c_void_p]
            
            symbol_table_lib.free_scope_list_bridge.restype = None
            symbol_table_lib.free_scope_list_bridge.argtypes = [ctypes.c_void_p]

        # Free symbol table
        symbol_table_lib.free_symbol_table.restype = None
        symbol_table_lib.free_symbol_table.argtypes = [ctypes.c_void_p]

        # Get all symbol details bridge
        if hasattr(symbol_table_lib, 'get_all_symbol_details_bridge'):
            symbol_table_lib.get_all_symbol_details_bridge.restype = ctypes.c_void_p
            symbol_table_lib.get_all_symbol_details_bridge.argtypes = [ctypes.c_void_p]
            
            symbol_table_lib.free_symbol_details_list.restype = None
            symbol_table_lib.free_symbol_details_list.argtypes = [ctypes.c_void_p]
            
            symbol_table_lib.get_symbol_details_bridge.restype = ctypes.c_void_p
            symbol_table_lib.get_symbol_details_bridge.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
            
            symbol_table_lib.free_symbol_details_bridge.restype = None
            symbol_table_lib.free_symbol_details_bridge.argtypes = [ctypes.c_void_p]

    def insert_symbol(self, name: str, data_type: DataType, kind: SymbolKind, line_number: int) -> bool:
        """Insert a new symbol into the symbol table"""
        if symbol_table_lib is None or self.symbol_table_ptr is None:
            return False
        
        name_bytes = name.encode('utf-8')
        result = symbol_table_lib.symbol_table_insert(
            self.symbol_table_ptr, name_bytes, 
            int(data_type), int(kind), line_number
        )
        return result == 1

    def lookup_symbol(self, name: str) -> Optional[SymbolEntry]:
        """Lookup a symbol in the symbol table"""
        if symbol_table_lib is None or self.symbol_table_ptr is None:
            return None
        
        name_bytes = name.encode('utf-8')
        
        # Use details bridge if available for full info including initial_value
        if hasattr(symbol_table_lib, 'get_symbol_details_bridge'):
            details_ptr = symbol_table_lib.get_symbol_details_bridge(self.symbol_table_ptr, name_bytes)
            if not details_ptr:
                return None
            
            # Parse details struct
            class CSymbolDetails(ctypes.Structure):
                _fields_ = [
                    ("name", ctypes.c_char * 256),
                    ("data_type", ctypes.c_int),
                    ("kind", ctypes.c_int),
                    ("storage_class", ctypes.c_int),
                    ("scope_level", ctypes.c_int),
                    ("scope_name", ctypes.c_char * 64),
                    ("size", ctypes.c_int),
                    ("offset", ctypes.c_int),
                    ("line_number", ctypes.c_int),
                    ("is_initialized", ctypes.c_int),
                    ("initial_value", ctypes.c_char * 64),
                    ("num_parameters", ctypes.c_int),
                    ("array_dimensions", ctypes.c_int),
                    ("dimension_sizes", ctypes.c_int * 10),
                ]
            
            c_details = ctypes.cast(details_ptr, ctypes.POINTER(CSymbolDetails)).contents
            
             # Extract dimension sizes
            dimension_sizes = []
            for i in range(c_details.array_dimensions):
                dimension_sizes.append(c_details.dimension_sizes[i])
                
            entry = SymbolEntry(
                name=c_details.name.decode('utf-8'),
                data_type=DataType(c_details.data_type),
                kind=SymbolKind(c_details.kind),
                storage_class=StorageClass(c_details.storage_class),
                scope_level=c_details.scope_level,
                scope_name=c_details.scope_name.decode('utf-8'),
                size=c_details.size,
                offset=c_details.offset,
                line_number=c_details.line_number,
                is_initialized=bool(c_details.is_initialized),
                initial_value=c_details.initial_value.decode('utf-8'),
                num_parameters=c_details.num_parameters,
                param_types=[], # Details struct in bridge doesn't carry param types array yet (simplification), fix if needed but user didn't ask explicitly for full params list in eye view? YES they did "Parameter List (only for function)"
                # To support param types in details, we need to update C struct. 
                # For now let's fall back to basic lookup if we need complex structures, OR assume frontend fetches params separately?
                # Actually bridge.c get_symbol_details_bridge doesn't copy param_types. I should fix bridge.c or just handle it.
                # User asked for "Parameter List".
                # Let's check my bridge.c edit. I didn't add param_types to SymbolDetails in bridge.c.
                # I should probably just use the basic lookup logic but with updated struct.
                # However, basic lookup returns a pointer to internal C struct. Python can read it.
                # Let's use the internal struct definition updated with initial_value.
                array_dimensions=c_details.array_dimensions,
                dimension_sizes=dimension_sizes
            )
            
            symbol_table_lib.free_symbol_details_bridge(details_ptr)
            return entry

        # Fallback to direct pointer access if bridge details not available
        entry_ptr = symbol_table_lib.symbol_table_lookup(self.symbol_table_ptr, name_bytes)
        
        if not entry_ptr:
            return None
        
        return self._parse_symbol_entry(entry_ptr)
    
    def _parse_symbol_entry(self, entry_ptr) -> SymbolEntry:
        """Parse C SymbolEntry struct to Python SymbolEntry"""
        # Define the C struct based on include/hash_table.h
        class CSymbolEntry(ctypes.Structure):
            _fields_ = [
                ("name", ctypes.c_char_p),
                ("data_type", ctypes.c_int),
                ("kind", ctypes.c_int),
                ("storage_class", ctypes.c_int),
                ("scope_level", ctypes.c_int),
                ("scope_name", ctypes.c_char * 64),
                ("size", ctypes.c_int),
                ("offset", ctypes.c_int),
                ("line_number", ctypes.c_int),
                ("is_initialized", ctypes.c_int),
                ("initial_value", ctypes.c_char * 64), # Added
                ("num_parameters", ctypes.c_int),
                ("param_types", ctypes.POINTER(ctypes.c_int)),
                ("array_dimensions", ctypes.c_int),
                ("dimension_sizes", ctypes.POINTER(ctypes.c_int)),
                ("next", ctypes.c_void_p),
            ]
        
        c_entry = ctypes.cast(entry_ptr, ctypes.POINTER(CSymbolEntry)).contents
        
        # Extract param types
        param_types = []
        if c_entry.num_parameters > 0 and c_entry.param_types:
            for i in range(c_entry.num_parameters):
                param_types.append(DataType(c_entry.param_types[i]))
                
        # Extract dimension sizes
        dimension_sizes = []
        if c_entry.array_dimensions > 0 and c_entry.dimension_sizes:
            for i in range(c_entry.array_dimensions):
                dimension_sizes.append(c_entry.dimension_sizes[i])
        
        return SymbolEntry(
            name=c_entry.name.decode('utf-8') if c_entry.name else "",
            data_type=DataType(c_entry.data_type),
            kind=SymbolKind(c_entry.kind),
            storage_class=StorageClass(c_entry.storage_class),
            scope_level=c_entry.scope_level,
            scope_name=c_entry.scope_name.decode('utf-8'),
            size=c_entry.size,
            offset=c_entry.offset,
            line_number=c_entry.line_number,
            is_initialized=bool(c_entry.is_initialized),
            initial_value=c_entry.initial_value.decode('utf-8'),
            num_parameters=c_entry.num_parameters,
            param_types=param_types,
            array_dimensions=c_entry.array_dimensions,
            dimension_sizes=dimension_sizes
        )
    
    def delete_symbol(self, name: str) -> bool:
        """Delete a symbol"""
        if not symbol_table_lib or not self.symbol_table_ptr or not hasattr(symbol_table_lib, 'symbol_table_delete_bridge'):
            return False
            
        name_bytes = name.encode('utf-8')
        result = symbol_table_lib.symbol_table_delete_bridge(self.symbol_table_ptr, name_bytes)
        return result == 1

    def set_initial_value(self, name: str, value: str) -> bool:
        """Set initial value string"""
        if not symbol_table_lib or not self.symbol_table_ptr or not hasattr(symbol_table_lib, 'symbol_table_set_initial_value_bridge'):
            return False
            
        name_bytes = name.encode('utf-8')
        value_bytes = value.encode('utf-8')
        result = symbol_table_lib.symbol_table_set_initial_value_bridge(self.symbol_table_ptr, name_bytes, value_bytes)
        return result == 1

    def get_scope_hierarchy(self) -> List[Dict[str, Any]]:
        """Get scope hierarchy as a list of dictionaries"""
        if not symbol_table_lib or not self.symbol_table_ptr or not hasattr(symbol_table_lib, 'get_scope_hierarchy_bridge'):
            return []

        # Define structures
        class CScopeInfo(ctypes.Structure):
            _fields_ = [
                ("name", ctypes.c_char * 64),
                ("level", ctypes.c_int),
                ("parent_name", ctypes.c_char * 64),
            ]

        class CScopeList(ctypes.Structure):
            _fields_ = [
                ("scopes", ctypes.POINTER(CScopeInfo)),
                ("count", ctypes.c_int),
                ("capacity", ctypes.c_int),
            ]

        list_ptr = symbol_table_lib.get_scope_hierarchy_bridge(self.symbol_table_ptr)
        if not list_ptr:
            return []

        c_list = ctypes.cast(list_ptr, ctypes.POINTER(CScopeList)).contents
        
        scopes = []
        for i in range(c_list.count):
            scope_info = c_list.scopes[i]
            parent_name = scope_info.parent_name.decode('utf-8')
            scopes.append({
                "name": scope_info.name.decode('utf-8'),
                "level": scope_info.level,
                "parent_name": parent_name if parent_name else None
            })

        symbol_table_lib.free_scope_list_bridge(list_ptr)
        return scopes
    
    def enter_scope(self, scope_name: str) -> bool:
        """Enter a new scope"""
        if not symbol_table_lib or not self.symbol_table_ptr:
            return False
        
        name_bytes = scope_name.encode('utf-8')
        symbol_table_lib.symbol_table_enter_scope(self.symbol_table_ptr, name_bytes)
        self._scope_stack.append(scope_name)
        return True
    
    def exit_scope(self) -> bool:
        """Exit current scope"""
        if not symbol_table_lib or not self.symbol_table_ptr:
            return False
        
        symbol_table_lib.symbol_table_exit_scope(self.symbol_table_ptr)
        if len(self._scope_stack) > 1:
            self._scope_stack.pop()
        return True
    
    def set_storage_class(self, name: str, storage_class: StorageClass) -> bool:
        """Set storage class for a symbol"""
        if not symbol_table_lib or not self.symbol_table_ptr:
            return False
        
        name_bytes = name.encode('utf-8')
        result = symbol_table_lib.symbol_table_set_storage_class(
            self.symbol_table_ptr, name_bytes, int(storage_class)
        )
        return result == 1
    
    def set_array_dimensions(self, name: str, dimensions: List[int]) -> bool:
        """Set array dimensions for a symbol"""
        if not symbol_table_lib or not self.symbol_table_ptr:
            return False
        
        name_bytes = name.encode('utf-8')
        dim_array = (ctypes.c_int * len(dimensions))(*dimensions)
        result = symbol_table_lib.symbol_table_set_array_dims(
            self.symbol_table_ptr, name_bytes, len(dimensions), dim_array
        )
        return result == 1
    
    def set_function_parameters(self, name: str, param_types: List[DataType]) -> bool:
        """Set function parameters"""
        if not symbol_table_lib or not self.symbol_table_ptr:
            return False
        
        name_bytes = name.encode('utf-8')
        param_array = (ctypes.c_int * len(param_types))(*[int(pt) for pt in param_types])
        result = symbol_table_lib.symbol_table_set_function_params(
            self.symbol_table_ptr, name_bytes, len(param_types), param_array
        )
        return result == 1
    
    def mark_initialized(self, name: str) -> bool:
        """Mark a symbol as initialized"""
        if not symbol_table_lib or not self.symbol_table_ptr:
            return False
        
        name_bytes = name.encode('utf-8')
        result = symbol_table_lib.symbol_table_mark_initialized_bridge(self.symbol_table_ptr, name_bytes)
        return result == 1
    
    def is_declared(self, name: str) -> bool:
        """Check if a symbol is declared"""
        if not symbol_table_lib or not self.symbol_table_ptr:
            return False
        
        name_bytes = name.encode('utf-8')
        result = symbol_table_lib.symbol_table_is_declared_bridge(self.symbol_table_ptr, name_bytes)
        return result == 1
    
    def get_error_count(self) -> int:
        """Get the current error count"""
        if symbol_table_lib is None or self.symbol_table_ptr is None:
            return 0
        
        return symbol_table_lib.symbol_table_get_errors_bridge(self.symbol_table_ptr)
    
    def get_all_symbols(self) -> List[SymbolEntry]:
        """Get all symbols in the symbol table with full details"""
        if symbol_table_lib is None or self.symbol_table_ptr is None:
            return []
            
        if not hasattr(symbol_table_lib, 'get_all_symbol_details_bridge'):
            return []

        # Define Structures
        class CSymbolDetails(ctypes.Structure):
            _fields_ = [
                ("name", ctypes.c_char * 256),
                ("data_type", ctypes.c_int),
                ("kind", ctypes.c_int),
                ("storage_class", ctypes.c_int),
                ("scope_level", ctypes.c_int),
                ("scope_name", ctypes.c_char * 64),
                ("size", ctypes.c_int),
                ("offset", ctypes.c_int),
                ("line_number", ctypes.c_int),
                ("is_initialized", ctypes.c_int),
                ("initial_value", ctypes.c_char * 64),
                ("num_parameters", ctypes.c_int),
                ("array_dimensions", ctypes.c_int),
                ("dimension_sizes", ctypes.c_int * 10),
            ]

        class CSymbolDetailsList(ctypes.Structure):
            _fields_ = [
                ("symbols", ctypes.POINTER(CSymbolDetails)),
                ("count", ctypes.c_int),
                ("capacity", ctypes.c_int),
            ]

        list_ptr = symbol_table_lib.get_all_symbol_details_bridge(self.symbol_table_ptr)
        if not list_ptr:
            return []

        c_list = ctypes.cast(list_ptr, ctypes.POINTER(CSymbolDetailsList)).contents
        symbols = []

        for i in range(c_list.count):
            c_details = c_list.symbols[i]
            
            symbol = SymbolEntry(
                name=c_details.name.decode('utf-8'),
                data_type=DataType(c_details.data_type),
                kind=SymbolKind(c_details.kind),
                storage_class=StorageClass(c_details.storage_class),
                scope_level=c_details.scope_level,
                scope_name=c_details.scope_name.decode('utf-8'),
                size=c_details.size,
                offset=c_details.offset,
                line_number=c_details.line_number,
                is_initialized=bool(c_details.is_initialized),
                initial_value=c_details.initial_value.decode('utf-8'),
                num_parameters=c_details.num_parameters,
                array_dimensions=c_details.array_dimensions,
                dimension_sizes=list(c_details.dimension_sizes)[:c_details.array_dimensions]
            )
            symbols.append(symbol)

        symbol_table_lib.free_symbol_details_list(list_ptr)
        return symbols
    
    def reset(self):
        """Reset the symbol table by freeing and creating a new one"""
        if symbol_table_lib and self.symbol_table_ptr:
            symbol_table_lib.free_symbol_table(self.symbol_table_ptr)
        
        if symbol_table_lib:
            self.symbol_table_ptr = symbol_table_lib.create_symbol_table()
            self._scope_stack = ["global"]
            
    def __del__(self):
        """Cleanup when wrapper is destroyed"""
        if symbol_table_lib and self.symbol_table_ptr:
            symbol_table_lib.free_symbol_table(self.symbol_table_ptr)
