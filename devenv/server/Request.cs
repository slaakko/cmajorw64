using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace server
{
    public delegate void WriteLineToOutputWindow(string line);

    public abstract class RequestHandler
    {
        public virtual void HandleCompileRequest(CompileRequest request) { }
        public virtual void HandleExitRequest(ExitRequest request) { }
        public virtual void HandleCleanRequest(CleanRequest request) { }
        public virtual void HandleExecuteRequest(ExecuteRequest request) { }
    }

    public abstract class Request
    {
        public abstract void Process(RequestHandler handler);
    }

    public class ExitRequest : Request
    {
        public override void Process(RequestHandler handler)
        {
            handler.HandleExitRequest(this);
        }
    }

    public class CompileRequest : Request
    {
        public CompileRequest(string filePath, string config, bool strictNothrow, bool emitLlvm, bool emitOptLlvm, bool linkWithDebugRuntime, bool linkWithMsLink, int optimizationLevel)
        {
            this.filePath = filePath;
            this.config = config;
            this.strictNothrow = strictNothrow;
            this.emitLlvm = emitLlvm;
            this.emitOptLlvm = emitOptLlvm;
            this.linkWithDebugRuntime = linkWithDebugRuntime;
            this.linkWithMsLink = linkWithMsLink;
            this.optimizationLevel = optimizationLevel;
        }
        public override void Process(RequestHandler handler)
        {
            handler.HandleCompileRequest(this);
        }
        public string FilePath
        {
            get { return filePath; }
        }
        public string Config
        {
            get { return config; }
        }
        public bool StrictNothrow
        {
            get { return strictNothrow; }
        }
        public bool EmitLlvm
        {
            get { return emitLlvm; }
        }
        public bool EmitOptLlvm
        {
            get { return emitOptLlvm; }
        }
        public bool LinkWithDebugRuntime
        {
            get { return linkWithDebugRuntime; }
        }
        public bool LinkWithMsLink
        {
            get { return linkWithMsLink; }
        }
        public int OptimizationLevel
        {
            get { return optimizationLevel; }
        }
        private string filePath;
        private string config;
        private bool strictNothrow;
        private bool emitLlvm;
        private bool emitOptLlvm;
        private bool linkWithDebugRuntime;
        private bool linkWithMsLink;
        private int optimizationLevel;
    }

    public class CleanRequest : Request
    {
        public CleanRequest(string filePath, string config)
        {
            this.filePath = filePath;
            this.config = config;
        }
        public override void Process(RequestHandler handler)
        {
            handler.HandleCleanRequest(this);
        }
        public string FilePath
        {
            get { return filePath; }
        }
        public string Config
        {
            get { return config; }
        }
        private string filePath;
        private string config;
    }

    public class ExecuteRequest : Request
    {
        public ExecuteRequest(string executablePath, string arguments)
        {
            this.executablePath = executablePath;
            this.arguments = arguments;
        }
        public string ExecutablePath
        {
            get { return executablePath; }
        }
        public string Arguments
        {
            get { return arguments; }
        }
        public override void Process(RequestHandler handler)
        {
            handler.HandleExecuteRequest(this);
        }
        private string executablePath;
        private string arguments;
    }
}
