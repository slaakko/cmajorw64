using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Build.Utilities;
using Microsoft.Build.Framework;
using System.IO;

namespace CmajorTasks
{
    public class ResolveModuleFilePath : Task
    {
        public override bool Execute()
        {
            if (string.IsNullOrEmpty(ProjectDir))
            {
                Log.LogError("'ProjectDir' is empty");
                return false;
            }
            if (string.IsNullOrEmpty(Configuration))
            {
                Log.LogError("'Configuration' is empty");
                return false;
            }
            int n = 0;
            if (ProjectReferences != null)
            {
                n = ProjectReferences.Length;
            }
            string config = "debug";
            if (Configuration == "Release")
            {
                config = "release";
            }
            ModuleFilePaths = new ITaskItem[n + 1];
            string cmajorRoot = Environment.GetEnvironmentVariable("CMAJOR_ROOT");
            string systemModuleFilePath = Path.GetFullPath(Path.Combine(Path.Combine(Path.Combine(Path.Combine(cmajorRoot, "system"), "lib"), config), "System.cmm"));
            ModuleFilePaths[0] = new TaskItem(systemModuleFilePath);
            for (int i = 0; i < n; ++i)
            {
                ITaskItem projectReferenceTaskITem = ProjectReferences[i];
                string projectReferencePath = projectReferenceTaskITem.ItemSpec;
                string moduleFilePath = Path.GetFullPath(Path.Combine(Path.Combine(Path.Combine(Path.Combine(ProjectDir, Path.GetDirectoryName(projectReferencePath)), "lib"), config), 
                    Path.GetFileNameWithoutExtension(projectReferencePath) + ".cmm"));
                ModuleFilePaths[i + 1] = new TaskItem(moduleFilePath);
            }
            return true;
        }

        public string ProjectDir { get; set; }

        public string Configuration { get; set; }

        public ITaskItem[] ProjectReferences { get; set; }

        [Output]
        public ITaskItem[] ModuleFilePaths { get; set; }
    }
}
