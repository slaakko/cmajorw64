using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Build.Utilities;
using Microsoft.Build.Framework;

namespace cmctask
{
    public class Cmc : Task
    {
        public override bool Execute()
        {
            Log.LogMessage(MessageImportance.High, "Cmajor Compile Task Executing !!!");
            return true;
        }

        public ITaskItem[] Sources { get; set; }

        public string Configuration { get; set; }

        public string TargetType { get; set; }
    }
}
