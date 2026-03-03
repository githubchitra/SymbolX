import React, { useState, useMemo } from 'react';
import {
    AlertCircle, AlertTriangle, Info, Lightbulb, Search,
    ChevronDown, ChevronRight, Folder, Hash, Tag, Target
} from 'lucide-react';

const ProblemsPanel = ({ problems = [], onNavigate }) => {
    const [filter, setFilter] = useState('all');
    const [searchTerm, setSearchTerm] = useState('');
    const [expandedGroups, setExpandedGroups] = useState(['global', 'errors', 'warnings']);

    const stats = useMemo(() => ({
        error: problems.filter(p => p.severity === 'error').length,
        warning: problems.filter(p => p.severity === 'warning').length,
        info: problems.filter(p => p.severity === 'info').length,
        suggestion: problems.filter(p => p.severity === 'suggestion').length,
    }), [problems]);

    const toggleGroup = (groupId) => {
        setExpandedGroups(prev =>
            prev.includes(groupId) ? prev.filter(id => id !== groupId) : [...prev, groupId]
        );
    };

    const filteredProblems = useMemo(() => {
        return problems.filter(p => {
            const matchesFilter = filter === 'all' || p.severity === filter;
            const matchesSearch = p.message.toLowerCase().includes(searchTerm.toLowerCase()) ||
                p.error_id?.toLowerCase().includes(searchTerm.toLowerCase()) ||
                p.symbol?.toLowerCase().includes(searchTerm.toLowerCase());
            return matchesFilter && matchesSearch;
        });
    }, [problems, filter, searchTerm]);

    // Group by scope or severity based on preference
    const groupedProblems = useMemo(() => {
        const groups = {};
        filteredProblems.forEach(p => {
            const g = p.scope || 'global';
            if (!groups[g]) groups[g] = [];
            groups[g].push(p);
        });
        return groups;
    }, [filteredProblems]);

    const getSeverityIcon = (severity) => {
        switch (severity) {
            case 'error': return <AlertCircle className="h-4 w-4 text-red-500" />;
            case 'warning': return <AlertTriangle className="h-4 w-4 text-amber-500" />;
            case 'info': return <Info className="h-4 w-4 text-blue-500" />;
            case 'suggestion': return <Lightbulb className="h-4 w-4 text-emerald-500" />;
            default: return null;
        }
    };

    const getSeverityBorder = (severity) => {
        switch (severity) {
            case 'error': return 'border-l-4 border-l-red-500';
            case 'warning': return 'border-l-4 border-l-amber-500';
            case 'info': return 'border-l-4 border-l-blue-500';
            case 'suggestion': return 'border-l-4 border-l-emerald-500';
            default: return '';
        }
    };

    return (
        <div className="bg-white rounded-xl shadow-2xl overflow-hidden flex flex-col h-full border border-gray-200 font-sans">
            {/* Premium Header */}
            <div className="bg-slate-900 text-white p-5 border-b border-white/10">
                <div className="flex flex-col md:flex-row md:items-center justify-between gap-4 mb-5">
                    <div className="flex items-center gap-3">
                        <div className="p-2 bg-blue-600 rounded-lg">
                            <Hash className="h-5 w-5 text-white" />
                        </div>
                        <div>
                            <h2 className="text-xl font-bold tracking-tight">Diagnostic Engine</h2>
                            <p className="text-xs text-slate-400">Real-time code analysis and validation</p>
                        </div>
                    </div>

                    <div className="flex gap-2 bg-slate-800 p-1 rounded-xl border border-slate-700">
                        <span className={`flex items-center gap-1.5 px-3 py-1 rounded-lg text-xs font-semibold ${stats.error > 0 ? 'bg-red-500/10 text-red-400 border border-red-500/20' : 'text-slate-500'}`}>
                            <AlertCircle className="h-3.5 w-3.5" /> {stats.error} Errors
                        </span>
                        <span className={`flex items-center gap-1.5 px-3 py-1 rounded-lg text-xs font-semibold ${stats.warning > 0 ? 'bg-amber-500/10 text-amber-400 border border-amber-500/20' : 'text-slate-500'}`}>
                            <AlertTriangle className="h-3.5 w-3.5" /> {stats.warning} Warnings
                        </span>
                        <div className="w-px h-4 bg-slate-700 self-center mx-1" />
                        <span className="flex items-center gap-1.5 px-3 py-1 text-slate-400 text-xs font-semibold">
                            <Info className="h-3.5 w-3.5" /> {stats.info + stats.suggestion} Tips
                        </span>
                    </div>
                </div>

                <div className="flex flex-col sm:flex-row gap-3">
                    <div className="relative flex-1">
                        <Search className="absolute left-3 top-1/2 -translate-y-1/2 h-4 w-4 text-slate-400" />
                        <input
                            type="text"
                            placeholder="Search by message, ID, or symbol..."
                            className="w-full pl-10 pr-4 py-2 bg-slate-800 border border-slate-700 rounded-xl text-sm text-white placeholder:text-slate-500 focus:outline-none focus:ring-2 focus:ring-blue-500 focus:border-transparent transition-all"
                            value={searchTerm}
                            onChange={(e) => setSearchTerm(e.target.value)}
                        />
                    </div>

                    <div className="flex gap-1 overflow-x-auto scrollbar-hide bg-slate-800 p-1 rounded-xl border border-slate-700">
                        {['all', 'error', 'warning', 'info'].map((f) => (
                            <button
                                key={f}
                                onClick={() => setFilter(f)}
                                className={`px-4 py-1.5 text-xs font-medium rounded-lg capitalize transition-all ${filter === f ? 'bg-blue-600 text-white shadow-lg' : 'text-slate-400 hover:text-white hover:bg-slate-700'
                                    }`}
                            >
                                {f}
                            </button>
                        ))}
                    </div>
                </div>
            </div>

            {/* Grouped List Content */}
            <div className="flex-1 overflow-y-auto bg-slate-50 p-4 space-y-4">
                {Object.keys(groupedProblems).length > 0 ? (
                    Object.entries(groupedProblems).map(([scope, items]) => (
                        <div key={scope} className="bg-white rounded-xl border border-gray-200 overflow-hidden shadow-sm">
                            <button
                                onClick={() => toggleGroup(scope)}
                                className="w-full flex items-center justify-between p-3 bg-gray-50 border-b border-gray-100 hover:bg-gray-100 transition-colors"
                            >
                                <div className="flex items-center gap-2">
                                    {expandedGroups.includes(scope) ? <ChevronDown className="h-4 w-4 text-gray-500" /> : <ChevronRight className="h-4 w-4 text-gray-500" />}
                                    <Folder className="h-4 w-4 text-blue-500" />
                                    <span className="text-sm font-bold text-gray-700">Scope: {scope}</span>
                                    <span className="ml-2 px-2 py-0.5 rounded-full bg-gray-200 text-gray-600 text-[10px] font-bold">
                                        {items.length} issue{items.length !== 1 ? 's' : ''}
                                    </span>
                                </div>
                            </button>

                            {expandedGroups.includes(scope) && (
                                <div className="divide-y divide-gray-100">
                                    {items.map((p) => (
                                        <div
                                            key={p.id}
                                            onClick={() => onNavigate && onNavigate(p.line)}
                                            className={`group p-4 flex items-start gap-4 transition-all cursor-pointer hover:bg-slate-50 ${getSeverityBorder(p.severity)}`}
                                        >
                                            <div className="mt-1 p-1 rounded-lg bg-white shadow-sm border border-gray-100 group-hover:scale-110 transition-transform">
                                                {getSeverityIcon(p.severity)}
                                            </div>

                                            <div className="flex-1">
                                                <div className="flex flex-wrap items-center gap-2 mb-1.5">
                                                    <span className="text-sm font-semibold text-gray-900 group-hover:text-blue-600 transition-colors">
                                                        {p.message}
                                                    </span>
                                                    <span className="text-[10px] items-center gap-1 font-mono uppercase font-bold text-slate-400 bg-slate-100 px-1.5 py-0.5 rounded border border-slate-200 flex">
                                                        <Tag className="h-2.5 w-2.5" /> {p.error_id}
                                                    </span>
                                                    {p.symbol && (
                                                        <span className="text-[10px] flex items-center gap-1 font-mono font-bold text-blue-500 bg-blue-50 px-1.5 py-0.5 rounded border border-blue-100">
                                                            <Target className="h-2.5 w-2.5" /> symbol: {p.symbol}
                                                        </span>
                                                    )}
                                                </div>

                                                {p.code && (
                                                    <div className="mt-2 mb-2 p-2 bg-slate-900 rounded-lg font-mono text-xs text-slate-300 border border-slate-800 overflow-x-auto">
                                                        <span className="text-slate-500 mr-2 select-none">{p.line} |</span>
                                                        {p.code}
                                                    </div>
                                                )}

                                                <div className="flex items-center justify-between mt-3">
                                                    <div className="flex items-center gap-4">
                                                        <div className="flex items-center gap-1.5 text-xs text-slate-500 font-medium bg-slate-100 px-2 py-1 rounded">
                                                            <Hash className="h-3 w-3" /> Line {p.line}{p.column > 0 ? `, Col ${p.column}` : ''}
                                                        </div>
                                                        {p.suggestion && (
                                                            <div className="flex items-center gap-1.5 text-xs text-emerald-600 font-medium">
                                                                <Lightbulb className="h-3 w-3" /> {p.suggestion}
                                                            </div>
                                                        )}
                                                    </div>

                                                    <button className="opacity-0 group-hover:opacity-100 bg-blue-600 text-white text-[10px] font-bold px-3 py-1.5 rounded-lg shadow-lg shadow-blue-500/30 transition-all hover:bg-blue-700 active:scale-95">
                                                        FIX ISSUES
                                                    </button>
                                                </div>
                                            </div>
                                        </div>
                                    ))}
                                </div>
                            )}
                        </div>
                    ))
                ) : (
                    <div className="h-full flex flex-col items-center justify-center py-20 bg-white rounded-2xl border-2 border-dashed border-gray-200">
                        <div className="relative mb-6">
                            <div className="absolute inset-0 bg-blue-400/20 blur-3xl rounded-full" />
                            <Info className="h-24 w-24 text-blue-500 relative animate-pulse" />
                        </div>
                        <h3 className="text-xl font-bold text-gray-800 mb-2">System Optimized</h3>
                        <p className="text-sm text-gray-500 px-10 text-center max-w-sm">
                            We couldn't find any issues in your current code. Your implementation follows established patterns and standards!
                        </p>
                    </div>
                )}
            </div>
        </div>
    );
};

export default ProblemsPanel;
