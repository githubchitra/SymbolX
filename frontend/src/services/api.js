import axios from 'axios';

const API_BASE_URL = process.env.REACT_APP_API_URL || 'http://localhost:8004';

const api = axios.create({
  baseURL: API_BASE_URL,
  headers: {
    'Content-Type': 'application/json',
  },
});

// Symbol table API endpoints
export const symbolTableAPI = {
  // Get all symbols
  getAllSymbols: async () => {
    const response = await api.get('/api/symbols');
    return response.data;
  },

  // Get specific symbol
  getSymbol: async (name) => {
    const response = await api.get(`/api/symbols/${name}`);
    return response.data;
  },

  // Create new symbol
  createSymbol: async (symbolData) => {
    const response = await api.post('/api/symbols', symbolData);
    return response.data;
  },

  // Update symbol
  updateSymbol: async (name, updateData) => {
    const response = await api.put(`/api/symbols/${name}`, updateData);
    return response.data;
  },

  // Delete symbol
  deleteSymbol: async (name) => {
    const response = await api.delete(`/api/symbols/${name}`);
    return response.data;
  },
};

// Scope management API endpoints
export const scopeAPI = {
  // Enter new scope
  enterScope: async (scopeName) => {
    const response = await api.post('/api/scope/enter', { scope_name: scopeName });
    return response.data;
  },

  // Exit current scope
  exitScope: async () => {
    const response = await api.post('/api/scope/exit');
    return response.data;
  },

  // Get scope hierarchy
  getScopeHierarchy: async () => {
    const response = await api.get('/api/scope/hierarchy');
    return response.data;
  },
};

// Code parsing API endpoints
export const parseAPI = {
  // Parse code
  parseCode: async (code, stepByStep = false) => {
    const response = await api.post('/api/parse', {
      code,
      step_by_step: stepByStep,
    });
    return response.data;
  },

  // Real-time analysis only
  analyzeCode: async (code) => {
    const response = await api.post('/api/analyze', { code });
    return response.data;
  },
};

// Export API endpoints
export const exportAPI = {
  // Export symbol table
  exportSymbolTable: async (format = 'json') => {
    const response = await api.get(`/api/export/${format}`);
    return response.data;
  },
};

// Statistics API endpoints
export const statsAPI = {
  // Get symbol table statistics
  getStats: async () => {
    const response = await api.get('/api/stats');
    return response.data;
  },
};

// WebSocket connection
export class WebSocketManager {
  constructor() {
    this.ws = null;
    this.listeners = new Map();
    this.reconnectAttempts = 0;
    this.maxReconnectAttempts = 5;
    this.reconnectDelay = 1000;
  }

  connect() {
    const wsUrl = API_BASE_URL.replace('http', 'ws') + '/ws';

    try {
      this.ws = new WebSocket(wsUrl);

      this.ws.onopen = () => {
        console.log('WebSocket connected');
        this.reconnectAttempts = 0;
        this.emit('connected');
      };

      this.ws.onmessage = (event) => {
        try {
          const data = JSON.parse(event.data);
          this.emit('message', data);
        } catch (error) {
          console.error('Error parsing WebSocket message:', error);
        }
      };

      this.ws.onclose = () => {
        console.log('WebSocket disconnected');
        this.emit('disconnected');
        this.attemptReconnect();
      };

      this.ws.onerror = (error) => {
        console.error('WebSocket error:', error);
        this.emit('error', error);
      };
    } catch (error) {
      console.error('Failed to connect WebSocket:', error);
      this.attemptReconnect();
    }
  }

  attemptReconnect() {
    if (this.reconnectAttempts < this.maxReconnectAttempts) {
      this.reconnectAttempts++;
      console.log(`Attempting to reconnect... (${this.reconnectAttempts}/${this.maxReconnectAttempts})`);

      setTimeout(() => {
        this.connect();
      }, this.reconnectDelay * this.reconnectAttempts);
    }
  }

  send(message) {
    if (this.ws && this.ws.readyState === WebSocket.OPEN) {
      this.ws.send(JSON.stringify(message));
    } else {
      console.warn('WebSocket is not connected');
    }
  }

  on(event, callback) {
    if (!this.listeners.has(event)) {
      this.listeners.set(event, []);
    }
    this.listeners.get(event).push(callback);
  }

  off(event, callback) {
    if (this.listeners.has(event)) {
      const callbacks = this.listeners.get(event);
      const index = callbacks.indexOf(callback);
      if (index > -1) {
        callbacks.splice(index, 1);
      }
    }
  }

  emit(event, data) {
    if (this.listeners.has(event)) {
      this.listeners.get(event).forEach(callback => {
        try {
          callback(data);
        } catch (error) {
          console.error('Error in WebSocket event callback:', error);
        }
      });
    }
  }

  disconnect() {
    if (this.ws) {
      this.ws.close();
      this.ws = null;
    }
  }
}

// Create global WebSocket manager instance
export const wsManager = new WebSocketManager();

// Error handling wrapper
export const withErrorHandling = async (apiCall, errorMessage = 'API call failed') => {
  try {
    return await apiCall();
  } catch (error) {
    if (error.response) {
      // Server responded with error status
      throw new Error(`${errorMessage}: ${error.response.data.detail || error.response.statusText}`);
    } else if (error.request) {
      // Request was made but no response received
      throw new Error(`${errorMessage}: Network error - no response received`);
    } else {
      // Something else happened
      throw new Error(`${errorMessage}: ${error.message}`);
    }
  }
};

export default api;
