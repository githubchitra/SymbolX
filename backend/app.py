from fastapi import FastAPI, HTTPException, WebSocket, WebSocketDisconnect
from fastapi.middleware.cors import CORSMiddleware
from fastapi.responses import JSONResponse
from pydantic import BaseModel
from typing import List, Optional, Dict, Any
import json
import asyncio
from symbol_table_wrapper import SymbolTableWrapper, DataType, SymbolKind, StorageClass
from enhanced_analyzer import EnhancedAnalyzer
import re

app = FastAPI(title="Symbol Table API", version="1.0.0")

# Enable CORS for frontend
app.add_middleware(
    CORSMiddleware,
    allow_origins=["http://localhost:3000", "http://localhost:5173"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# Global symbol table instance
symbol_table = SymbolTableWrapper()
analyzer = EnhancedAnalyzer()
websocket_connections = []

# Pydantic models for API
class SymbolInsertRequest(BaseModel):
    name: str
    data_type: str
    kind: str
    line_number: int
    storage_class: Optional[str] = "auto"
    array_dimensions: Optional[List[int]] = None
    function_parameters: Optional[List[str]] = None

class SymbolUpdateRequest(BaseModel):
    data_type: Optional[str] = None
    storage_class: Optional[str] = None
    is_initialized: Optional[bool] = None
    initial_value: Optional[str] = None # Added
    array_dimensions: Optional[List[int]] = None
    function_parameters: Optional[List[str]] = None

class ScopeEnterRequest(BaseModel):
    scope_name: str

class CodeParseRequest(BaseModel):
    code: str
    step_by_step: Optional[bool] = False

# WebSocket connection manager
class ConnectionManager:
    def __init__(self):
        self.active_connections: List[WebSocket] = []

    async def connect(self, websocket: WebSocket):
        await websocket.accept()
        self.active_connections.append(websocket)

    def disconnect(self, websocket: WebSocket):
        self.active_connections.remove(websocket)

    async def send_personal_message(self, message: str, websocket: WebSocket):
        await websocket.send_text(message)

    async def broadcast(self, message: str):
        for connection in self.active_connections:
            await connection.send_text(message)

manager = ConnectionManager()

# Helper functions
def parse_data_type(data_type_str: str) -> DataType:
    type_mapping = {
        "int": DataType.TYPE_INT,
        "float": DataType.TYPE_FLOAT,
        "char": DataType.TYPE_CHAR,
        "double": DataType.TYPE_DOUBLE,
        "void": DataType.TYPE_VOID,
        "bool": DataType.TYPE_BOOL,
        "string": DataType.TYPE_STRING,
        "array": DataType.TYPE_ARRAY,
        "pointer": DataType.TYPE_POINTER,
        "struct": DataType.TYPE_STRUCT,
        "function": DataType.TYPE_FUNCTION
    }
    return type_mapping.get(data_type_str.lower(), DataType.TYPE_UNKNOWN)

def parse_symbol_kind(kind_str: str) -> SymbolKind:
    kind_mapping = {
        "variable": SymbolKind.SYMBOL_VARIABLE,
        "function": SymbolKind.SYMBOL_FUNCTION,
        "parameter": SymbolKind.SYMBOL_PARAMETER,
        "constant": SymbolKind.SYMBOL_CONSTANT,
        "array": SymbolKind.SYMBOL_ARRAY,
        "struct": SymbolKind.SYMBOL_STRUCT,
        "typedef": SymbolKind.SYMBOL_TYPEDEF
    }
    return kind_mapping.get(kind_str.lower(), SymbolKind.SYMBOL_VARIABLE)

def parse_storage_class(storage_str: str) -> StorageClass:
    storage_mapping = {
        "auto": StorageClass.STORAGE_AUTO,
        "static": StorageClass.STORAGE_STATIC,
        "extern": StorageClass.STORAGE_EXTERN,
        "register": StorageClass.STORAGE_REGISTER
    }
    return storage_mapping.get(storage_str.lower(), StorageClass.STORAGE_AUTO)

def symbol_to_dict(symbol) -> Optional[Dict[str, Any]]:
    """Convert SymbolEntry to dictionary for JSON response"""
    if not symbol:
        return None
    
    # Strip TYPE_ and SYMBOL_ prefixes for better display
    data_type = symbol.data_type.name.replace('TYPE_', '').lower()
    kind = symbol.kind.name.replace('SYMBOL_', '').lower()
    storage_class = symbol.storage_class.name.replace('STORAGE_', '').lower()
    
    status = "declared"
    if symbol.kind == SymbolKind.SYMBOL_FUNCTION:
        status = "defined"
    elif symbol.kind == SymbolKind.SYMBOL_PARAMETER:
        status = "parameter"
    elif symbol.is_initialized:
        status = "initialized"
    
    return {
        "name": symbol.name,
        "data_type": data_type,
        "kind": kind,
        "storage_class": storage_class,
        "scope_level": symbol.scope_level,
        "scope_name": symbol.scope_name,
        "size": symbol.size,
        "offset": symbol.offset,
        "line_number": symbol.line_number,
        "is_initialized": symbol.is_initialized,
        "initial_value": symbol.initial_value,
        "status": status,
        "num_parameters": symbol.num_parameters,
        "param_types": [pt.name.replace('TYPE_', '').lower() for pt in symbol.param_types],
        "array_dimensions": symbol.array_dimensions,
        "dimension_sizes": symbol.dimension_sizes
    }

# API Routes
@app.get("/")
async def root():
    return {"message": "Symbol Table API", "version": "1.0.0"}

@app.get("/api/symbols")
async def get_all_symbols():
    """Get all symbols in the symbol table"""
    try:
        symbols = symbol_table.get_all_symbols()
        return {"symbols": [symbol_to_dict(symbol) for symbol in symbols]}
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@app.get("/api/symbols/{name}")
async def get_symbol(name: str):
    """Get a specific symbol by name"""
    try:
        symbol = symbol_table.lookup_symbol(name)
        if not symbol:
            raise HTTPException(status_code=404, detail=f"Symbol '{name}' not found")
        return symbol_to_dict(symbol)
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@app.post("/api/symbols")
async def create_symbol(request: SymbolInsertRequest):
    """Create a new symbol"""
    try:
        data_type = parse_data_type(request.data_type)
        kind = parse_symbol_kind(request.kind)
        
        # Insert the symbol
        success = symbol_table.insert_symbol(
            request.name, data_type, kind, request.line_number
        )
        
        if not success:
            raise HTTPException(status_code=400, detail="Failed to insert symbol (possibly duplicate)")
        
        # Set additional properties
        if request.storage_class:
            storage_class = parse_storage_class(request.storage_class)
            symbol_table.set_storage_class(request.name, storage_class)
        
        if request.array_dimensions is not None and kind == SymbolKind.SYMBOL_ARRAY:
            symbol_table.set_array_dimensions(request.name, request.array_dimensions)
        
        if request.function_parameters is not None and kind == SymbolKind.SYMBOL_FUNCTION:
            param_types = [parse_data_type(pt) for pt in request.function_parameters]
            symbol_table.set_function_parameters(request.name, param_types)
        
        # Get the created symbol
        symbol = symbol_table.lookup_symbol(request.name)
        
        # Broadcast update to WebSocket clients
        await manager.broadcast(json.dumps({
            "type": "symbol_added",
            "symbol": symbol_to_dict(symbol)
        }))
        
        return {"message": "Symbol created successfully", "symbol": symbol_to_dict(symbol)}
    
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@app.put("/api/symbols/{name}")
async def update_symbol(name: str, request: SymbolUpdateRequest):
    """Update an existing symbol"""
    try:
        symbol = symbol_table.lookup_symbol(name)
        if not symbol:
            raise HTTPException(status_code=404, detail=f"Symbol '{name}' not found")
        
        # Update properties
        if request.storage_class:
            storage_class = parse_storage_class(request.storage_class)
            symbol_table.set_storage_class(name, storage_class)
        
        if request.is_initialized is not None:
            if request.is_initialized:
                symbol_table.mark_initialized(name)
        
        if request.initial_value is not None:
             symbol_table.set_initial_value(name, request.initial_value)

        if request.array_dimensions is not None:
            symbol_table.set_array_dimensions(name, request.array_dimensions)
        
        if request.function_parameters is not None:
            param_types = [parse_data_type(pt) for pt in request.function_parameters]
            symbol_table.set_function_parameters(name, param_types)
        
        # Get updated symbol
        updated_symbol = symbol_table.lookup_symbol(name)
        
        # Broadcast update
        await manager.broadcast(json.dumps({
            "type": "symbol_updated",
            "symbol": symbol_to_dict(updated_symbol)
        }))
        
        return {"message": "Symbol updated successfully", "symbol": symbol_to_dict(updated_symbol)}
    
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@app.delete("/api/symbols/{name}")
async def delete_symbol(name: str):
    """Delete a symbol"""
    try:
        success = symbol_table.delete_symbol(name)
        if not success:
            raise HTTPException(status_code=404, detail=f"Symbol '{name}' not found or could not be deleted")
        
        # Broadcast delete
        await manager.broadcast(json.dumps({
            "type": "symbol_deleted",
            "symbol_name": name
        }))
        
        return {"message": "Symbol deleted successfully"}
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@app.post("/api/scope/enter")
async def enter_scope(request: ScopeEnterRequest):
    """Enter a new scope"""
    try:
        success = symbol_table.enter_scope(request.scope_name)
        if not success:
            raise HTTPException(status_code=400, detail="Failed to enter scope")
        
        await manager.broadcast(json.dumps({
            "type": "scope_entered",
            "scope_name": request.scope_name
        }))
        
        return {"message": f"Entered scope '{request.scope_name}'"}
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@app.post("/api/scope/exit")
async def exit_scope():
    """Exit current scope"""
    try:
        success = symbol_table.exit_scope()
        if not success:
            raise HTTPException(status_code=400, detail="Failed to exit scope")
        
        await manager.broadcast(json.dumps({
            "type": "scope_exited"
        }))
        
        return {"message": "Exited current scope"}
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@app.get("/api/scope/hierarchy")
async def get_scope_hierarchy():
    """Get the scope hierarchy tree as requested"""
    try:
        # Get scopes from the wrapper (which uses the C library hierarchy)
        flat_scopes = symbol_table.get_scope_hierarchy()
        all_symbols = symbol_table.get_all_symbols()
        
        # Map to quickly find scope objects by name
        scope_map = {}
        
        # 1. Create all scope objects
        for s in flat_scopes:
            scope_map[s["name"]] = {
                "name": s["name"],
                "level": s["level"],
                "symbols": [],
                "children": [],
                "symbol_count": 0,
                "parent_name": s["parent_name"]
            }
            
        # 2. Assign symbols to their respective scopes
        for sym in all_symbols:
            if sym.scope_name in scope_map:
                scope_map[sym.scope_name]["symbols"].append(sym.name)
                scope_map[sym.scope_name]["symbol_count"] += 1
                
        # 3. Link children to their respective parents
        for name, scope in scope_map.items():
            parent_name = scope.get("parent_name")
            # Only link if parent exists, is in our map, and is not ourselves
            if parent_name and parent_name in scope_map and parent_name != name:
                scope_map[parent_name]["children"].append(scope)
                
        # 4. Identify the root nodes (those without a parent in our map)
        root_scopes = []
        for name, scope in scope_map.items():
            parent_name = scope.get("parent_name")
            if not parent_name or parent_name not in scope_map:
                root_scopes.append(scope)
                
        # If the list is empty, ensure at least global is there
        if not root_scopes and "global" in scope_map:
            root_scopes = [scope_map["global"]]
        elif not root_scopes:
            root_scopes = [{
                "name": "global",
                "level": 0,
                "symbols": [],
                "children": [],
                "symbol_count": 0
            }]

        return {
            "scopes": root_scopes,
            "current_scope": symbol_table.current_scope_name
        }
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@app.post("/api/analyze")
async def analyze_code(request: CodeParseRequest):
    """Real-time code analysis without updating symbol table state"""
    try:
        problems = analyzer.analyze(request.code)
        return {"problems": problems}
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@app.post("/api/parse")
async def parse_code(request: CodeParseRequest):
    """Parse code and update symbol table using strict rules"""
    try:
        lines = request.code.split('\n')
        results = []
        brace_level = 0
        function_stack = [] # Stack of brace levels where functions were entered
        scope_counter = 0 # Counter to ensure unique scope names
        
        # Reset symbol table before parsing new code
        symbol_table.reset()
        
        # Valid C types for the parser
        valid_types = ["int", "float", "char", "double", "void", "long", "short", "bool"]
        type_pattern = r'\b(' + '|'.join(valid_types) + r')\b'
        
        for i, line_content in enumerate(lines, 1):
            # 1. Strip comments
            clean_line = line_content.split('//')[0]
            if not clean_line.strip():
                # Even in empty-ish lines, we must process braces
                for char in clean_line:
                    if char == '{':
                        scope_counter += 1
                        symbol_table.enter_scope(f"block_L{i}_{scope_counter}")
                        brace_level += 1
                    elif char == '}':
                        symbol_table.exit_scope()
                        brace_level = max(0, brace_level - 1)
                        if function_stack and brace_level == function_stack[-1]:
                            symbol_table.exit_scope()
                            function_stack.pop()
                continue

            # 2. Gather all events (type declarations and braces) in the line
            events = []
            
            # Find type matches
            for match in re.finditer(type_pattern, clean_line):
                events.append({
                    "pos": match.start(),
                    "type": "decl",
                    "match": match
                })
            
            # Find braces
            for idx, char in enumerate(clean_line):
                if char == '{':
                    events.append({"pos": idx, "type": "brace_open"})
                elif char == '}':
                    events.append({"pos": idx, "type": "brace_close"})
            
            # Sort events by their position in the line
            events.sort(key=lambda x: x["pos"])
            
            # 3. Process events in order
            for event in events:
                if event["type"] == "brace_open":
                    scope_counter += 1
                    symbol_table.enter_scope(f"block_L{i}_{scope_counter}")
                    brace_level += 1
                
                elif event["type"] == "brace_close":
                    symbol_table.exit_scope() # Exit block scope
                    brace_level = max(0, brace_level - 1)
                    
                    # If we returned to the level where a function was entered, exit function scope too
                    if function_stack and brace_level == function_stack[-1]:
                        symbol_table.exit_scope()
                        function_stack.pop()
                
                elif event["type"] == "decl":
                    type_match = event["match"]
                    data_type_str = type_match.group(1)
                    data_type = parse_data_type(data_type_str)
                    
                    # Get text after the type
                    remainder = clean_line[type_match.end():].strip()
                    if not remainder or remainder.startswith(')'):
                        continue
                    
                    # Handle the first identifier
                    ident_match = re.search(r'^([a-zA-Z_][a-zA-Z0-9_]*)\s*([\(;=,])?', remainder)
                    if ident_match:
                        ident_name = ident_match.group(1)
                        suffix = ident_match.group(2)
                        
                        is_function = (suffix == '(')
                        kind = SymbolKind.SYMBOL_FUNCTION if is_function else SymbolKind.SYMBOL_VARIABLE
                        
                        # Insert the symbol
                        if symbol_table.insert_symbol(ident_name, data_type, kind, i):
                            if not is_function and suffix == '=':
                                symbol_table.mark_initialized(ident_name)
                            
                            results.append({
                                "line": i,
                                "symbol": ident_name,
                                "type": data_type_str,
                                "kind": "function" if is_function else "variable",
                                "scope": symbol_table.current_scope_name,
                                "status": "defined" if is_function else ("initialized" if suffix == '=' else "declared")
                            })
                            
                            # If it's a function, enter its scope for its parameters
                            if is_function:
                                symbol_table.enter_scope(f"function:{ident_name}")
                                function_stack.append(brace_level)
                            
                            # Handle multiple declarations on one line (int a, b;)
                            if not is_function and suffix == ',':
                                for part in remainder.split(',')[1:]:
                                    part = part.strip()
                                    m = re.search(r'^([a-zA-Z_][a-zA-Z0-9_]*)\s*([;=,])?', part)
                                    if m:
                                        s_name = m.group(1)
                                        s_suffix = m.group(2)
                                        if symbol_table.insert_symbol(s_name, data_type, SymbolKind.SYMBOL_VARIABLE, i):
                                            if s_suffix == '=':
                                                symbol_table.mark_initialized(s_name)
                                            results.append({
                                                "line": i,
                                                "symbol": s_name,
                                                "type": data_type_str,
                                                "kind": "variable",
                                                "scope": symbol_table.current_scope_name,
                                                "status": "initialized" if s_suffix == '=' else "declared"
                                            })
                                        if s_suffix != ',': break

        # Run enhanced analysis
        problems = analyzer.analyze(request.code)

        return {
            "results": results, 
            "errors": symbol_table.get_error_count(),
            "problems": problems
        }
    
    except Exception as e:
        # Use simple string for detail to avoid complex object serialization issues
        raise HTTPException(status_code=500, detail=f"Parser Error: {str(e)}")

@app.get("/api/export/{format}")
async def export_symbol_table(format: str):
    """Export symbol table in specified format"""
    try:
        symbols = symbol_table.get_all_symbols()
        
        if format.lower() == "json":
            return {"symbols": [symbol_to_dict(symbol) for symbol in symbols]}
        elif format.lower() == "csv":
            # Simple CSV export
            csv_lines = ["Name,Type,Kind,Scope,Line,Initialized"]
            for symbol in symbols:
                csv_lines.append(f"{symbol.name},{symbol.data_type.name},{symbol.kind.name},{symbol.scope_name},{symbol.line_number},{symbol.is_initialized}")
            return {"csv": "\n".join(csv_lines)}
        else:
            raise HTTPException(status_code=400, detail="Unsupported format. Use 'json' or 'csv'")
    
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@app.get("/api/stats")
async def get_symbol_table_stats():
    """Get statistics about the symbol table"""
    try:
        symbols = symbol_table.get_all_symbols()
        
        # Explicitly initialize stats with proper dictionaries
        stats: Dict[str, Any] = {
            "total_symbols": len(symbols),
            "error_count": symbol_table.get_error_count(),
            "symbols_by_type": {},
            "symbols_by_kind": {},
            "symbols_by_scope": {}
        }
        
        symbols_by_type: Dict[str, int] = {}
        symbols_by_kind: Dict[str, int] = {}
        symbols_by_scope: Dict[str, int] = {}
        
        for symbol in symbols:
            # Count by type
            type_name = symbol.data_type.name
            symbols_by_type[type_name] = symbols_by_type.get(type_name, 0) + 1
            
            # Count by kind
            kind_name = symbol.kind.name
            symbols_by_kind[kind_name] = symbols_by_kind.get(kind_name, 0) + 1
            
            # Count by scope
            scope_name = symbol.scope_name
            symbols_by_scope[scope_name] = symbols_by_scope.get(scope_name, 0) + 1
            
        stats["symbols_by_type"] = symbols_by_type
        stats["symbols_by_kind"] = symbols_by_kind
        stats["symbols_by_scope"] = symbols_by_scope
        
        return stats
    
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

# WebSocket endpoint
@app.websocket("/ws")
async def websocket_endpoint(websocket: WebSocket):
    await manager.connect(websocket)
    try:
        while True:
            data = await websocket.receive_text()
            # Echo back or handle WebSocket messages
            await manager.send_personal_message(f"Received: {data}", websocket)
    except WebSocketDisconnect:
        manager.disconnect(websocket)

if __name__ == "__main__":
    import uvicorn
    uvicorn.run(app, host="0.0.0.0", port=8004)
