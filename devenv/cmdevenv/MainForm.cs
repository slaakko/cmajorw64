using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.IO;
using devcore;
using parser;
using server;

namespace cmdevenv
{
    public enum State
    {
        editing, compiling, running, debugging
    }

    public partial class MainForm : Form
    {
        public MainForm()
        {
            InitializeComponent();
            progressChars = "|/-\\";
            progressIndex = 0;
            editorTabControl = new XTabControl();
            editorTabControl.Dock = System.Windows.Forms.DockStyle.Fill;
            editorTabControl.Location = new System.Drawing.Point(0, 0);
            editorTabControl.Name = "editorTabControl";
            editorTabControl.SelectedIndex = 0;
            editorTabControl.Size = new System.Drawing.Size(400, 325);
            editorTabControl.TabIndex = 0;
            editorTabControl.TabClosing += editorTabControl_TabClosing;
            editorSplitContainer.Panel1.Controls.Add(editorTabControl);
            configComboBox.SelectedIndex = 0;
            compiler = new Compiler();
            compiler.CmcPath = Configuration.Instance.CmcPath;
            compiler.SetWriteMethod(this, WriteOutputLine);
            compiler.SetHandleCompileResultMethod(this, HandleCompileResult);
            executor = new Executor();
            executor.SetWriteMethod(this, WriteToConsole);
            executor.SetExecuteReadyMethod(this, ProjectRun);
            console = new ConsoleWindow(executor);
            consoleTabPage.Controls.Add(console);
            processRunning = false;
            buildInProgress = false;
            compileAborted = false;
            emitLlvm = Configuration.Instance.EmitLlvm; ;
            emitOptLlvm = Configuration.Instance.EmitOptLlvm;
            optimizationLevel = -1;
            errorListView.ContextMenuStrip = errorsListViewContextMenuStrip;
            string[] args = Environment.GetCommandLineArgs();
            if (args.Length > 1)
            {
                OpenProjectOrSolution(args[1]);
            }
        }
        private void aboutToolStripMenuItem_Click(object sender, EventArgs e)
        {
            try
            {
                AboutBox aboutBox = new AboutBox();
                aboutBox.ShowDialog();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }
        private async void openProjectSolutionToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (openProjectDialog.ShowDialog() == DialogResult.OK)
            {
                await OpenProjectOrSolution(openProjectDialog.FileName);
            }
        }
        private async Task OpenProjectOrSolution(string projectOrSolutionName)
        {
            try
            {
                if (solution != null)
                {
                    if (!await CloseSolution())
                    {
                        return;
                    }
                }
                string solutionFileName = projectOrSolutionName.EndsWith(".cms") ? projectOrSolutionName : projectOrSolutionName + ".cms";
                string solutionDir = Path.GetDirectoryName(solutionFileName);
                Directory.SetCurrentDirectory(solutionDir);
                if (File.Exists(solutionFileName))
                {
                    Configuration.Instance.AddRecentProject(solutionFileName);
                    Configuration.Instance.Save();
                    SetupRecentProjectMenu();
                    SolutionFile solutionFileParser = ParserRepository.Instance.SolutionFileParser;
                    solution = solutionFileParser.Parse(File.ReadAllText(solutionFileName), 0, solutionFileName);
                }
                else if (projectOrSolutionName.EndsWith(".cmp"))
                {
                    ProjectFile projectFileParser = ParserRepository.Instance.ProjectFileParser;
                    Project project = projectFileParser.Parse(File.ReadAllText(projectOrSolutionName), 0, projectOrSolutionName);
                    solution = new Solution(project.Name, Path.ChangeExtension(project.FilePath, ".cms"));
                    solution.AddProject(project);
                    solution.Save();
                    Configuration.Instance.AddRecentProject(solution.FilePath);
                    Configuration.Instance.Save();
                    SetupRecentProjectMenu();
                }
                await SetupSolutionExplorer(null);
                SetMenuItemStatus();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }
        private async Task SetupSolutionExplorer(Project visibleProject)
        {
            TreeNode visibleProjectNode = null;
            solutionExplorerTreeView.Nodes.Clear();
            int n = solution.Projects.Count;
            if (n > 0 && solution.ActiveProject == null)
            {
                solution.ActiveProject = solution.Projects[0];
            }
            TreeNode solutionNode = solutionExplorerTreeView.Nodes.Add("Solution '" + solution.Name + "' (" + n.ToString() + " project" + (n == 1 ? ")" : "s)"));
            solutionNode.ContextMenuStrip = solutionContextMenuStrip;
            foreach (Project project in solution.Projects)
            {
                TreeNode projectNode = solutionNode.Nodes.Add(project.Name);
                if (project == visibleProject)
                {
                    visibleProjectNode = projectNode;
                }
                projectNode.Tag = project;
                projectNode.ContextMenuStrip = projectContextMenuStrip;
                if (project == solution.ActiveProject)
                {
                    projectNode.NodeFont = new Font(solutionExplorerTreeView.Font, FontStyle.Bold);
                }
                foreach (SourceFile sourceFile in project.SourceFiles)
                {
                    TreeNode sourceNode = projectNode.Nodes.Add(sourceFile.Name);
                    sourceNode.Tag = sourceFile;
                    sourceNode.ContextMenuStrip = sourceFileContextMenuStrip;
                }
            }
            solutionExplorerTreeView.ExpandAll();
            if (visibleProjectNode != null)
            {
                visibleProjectNode.EnsureVisible();
            }
            else
            {
                solutionNode.EnsureVisible();
            }
        }
        private bool editorTabControl_TabClosing(EventArgs e)
        {
            try
            {
                if (editorTabControl.SelectedIndex != -1)
                {
                    TabPage selectedTab = editorTabControl.SelectedTab;
                    Editor editor = (Editor)selectedTab.Tag;
                    DialogResult result = editor.Close();
                    if (result == DialogResult.Cancel)
                    {
                        return false;
                    }
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
            return true;
        }
        public void SetMenuItemStatus()
        {
            bool editing = state == State.editing;
            bool debugging = state == State.debugging;
            bool editorOpen = editorTabControl.TabPages.Count > 0;
            bool solutionOpen = solution != null;
            bool activeProjectSet = solution != null && solution.ActiveProject != null;
            bool solutionHasProjects = solution != null && solution.Projects.Count > 0;
            closeToolStripMenuItem.Enabled = editing && editorOpen;
            closeSolutionToolStripMenuItem.Enabled = editing && solutionOpen;
            saveToolStripMenuItem.Enabled = editing && editorOpen;
            saveAllToolStripMenuItem.Enabled = editing && solutionOpen;
            batchBuildToolStripMenuItem.Enabled = editing && solutionHasProjects;
            buildSolutionToolStripMenuItem.Enabled = editing && solutionHasProjects;
            buildToolStripMenuItem1.Enabled = editing && solutionHasProjects;
            buildActiveProjectToolStripMenuItem.Enabled = editing && activeProjectSet;
            buildSolutionToolStripMenuItem.Enabled = editing && solutionHasProjects;
            cleanSolutionToolStripMenuItem.Enabled = editing && solutionHasProjects;
            cleanSolutionToolStripMenuItem2.Enabled = editing && solutionHasProjects;
            cleanActiveProjectToolStripMenuItem.Enabled = editing && activeProjectSet;
            cleanProjectToolStripMenuItem.Enabled = editing && solutionHasProjects;
            optionsToolStripMenuItem.Enabled = editing && solutionOpen;
            setAsActiveProjectToolStripMenuItem.Enabled = editing && solutionHasProjects;
            projectDependenciesToolStripMenuItem.Enabled = editing && solutionHasProjects;
            projectBuildOrderToolStripMenuItem.Enabled = editing && solutionHasProjects;
            projectDependenciesToolStripMenuItem1.Enabled = editing && solutionHasProjects;
            projectBuildOrderToolStripMenuItem1.Enabled = editing && solutionHasProjects;
            gotoLineToolStripMenuItem.Enabled = editorOpen;
            formatContentToolStripMenuItem.Enabled = editing && editorOpen;
            findToolStripMenuItem.Enabled = activeProjectSet && solution.ActiveProject.SourceFiles.Count > 0;
        }
        private async Task<bool> CloseSolution()
        {
            try
            {
                solution.Save();
                foreach (TabPage editorTab in editorTabControl.TabPages)
                {
                    Editor editor = (Editor)editorTab.Tag;
                    DialogResult result = editor.Close();
                    if (result == DialogResult.Cancel)
                    {
                        return false;
                    }
                    editorTabControl.TabPages.Remove(editorTab);
                }
                solutionExplorerTreeView.Nodes.Clear();
                solution = null;
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
            return true;
        }
        private void SetState(State state)
        {
            this.state = state;
            SetMenuItemStatus();
        }
        private void MainForm_Shown(object sender, EventArgs e)
        {
            try
            {
                alterConfig = false;
                SetState(State.editing);
                Top = Math.Max(0, Configuration.Instance.FormTop);
                Left = Math.Max(0, Configuration.Instance.FormLeft);
                Width = Math.Max(0, Configuration.Instance.FormSizeX);
                Height = Math.Max(0, Configuration.Instance.FormSizeY);
                string windowState = Configuration.Instance.WindowState;
                WindowState = windowState == "Normal" ? FormWindowState.Normal : windowState == "Maximized" ? FormWindowState.Maximized : FormWindowState.Normal;
                if (Configuration.Instance.ToolColWidth != 0) toolColumnHeader.Width = Configuration.Instance.ToolColWidth;
                if (Configuration.Instance.CategoryColWidth != 0) categoryColumnHeader.Width = Configuration.Instance.CategoryColWidth;
                if (Configuration.Instance.DescriptionColWidth != 0) descriptionColumnHeader.Width = Configuration.Instance.DescriptionColWidth;
                if (Configuration.Instance.FileColWidth != 0) fileColumnHeader.Width = Configuration.Instance.FileColWidth;
                if (Configuration.Instance.LineColWidth != 0) lineColumnHeader.Width = Configuration.Instance.LineColWidth;
                if (Configuration.Instance.ProjectColWidth != 0) projectColumnHeader.Width = Configuration.Instance.ProjectColWidth;
                if (Configuration.Instance.TextColWidth != 0) textColumnHeader.Width = Configuration.Instance.TextColWidth;
                if (Configuration.Instance.FindFileColWidth != 0) findFileColumnHeader.Width = Configuration.Instance.FindFileColWidth;
                if (Configuration.Instance.FindLineColWidth != 0) findLineColumnHeader.Width = Configuration.Instance.FindLineColWidth;
                if (Configuration.Instance.FindProjectColWidth != 0) findProjectColumnHeader.Width = Configuration.Instance.FindProjectColWidth;
                if (Configuration.Instance.FindTextColWidth != 0) findTextColumnHeader.Width = Configuration.Instance.FindTextColWidth;
                editorSplitContainer.SplitterDistance = Configuration.Instance.HorizontalSplitterPos;
                splitter1.SplitPosition = Configuration.Instance.VerticalSplitterPos;
                Resize += new EventHandler(MainForm_Resize);
                Move += new EventHandler(MainForm_Move);
                SetupRecentProjectMenu();
                infoTimer.Interval = 1000 * Configuration.Instance.InfoDelaySecs;
                infoTimer.Tick += infoTimer_Tick;
                alterConfig = true;
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }
        private void SetupRecentProjectMenu()
        {
            try
            {
                recentProjectsToolStripMenuItem.DropDownItems.Clear();
                List<string> recentProjects = Configuration.Instance.GetRecentProjectPaths();
                int n = recentProjects.Count;
                recentProjectsToolStripMenuItem.Enabled = n != 0;
                clearRecentProjectsToolStripMenuItem.Enabled = n != 0;
                for (int i = 0; i < n; ++i)
                {
                    ToolStripMenuItem recentProjectItem = new ToolStripMenuItem((i + 1).ToString() + " " + recentProjects[i]);
                    recentProjectItem.Tag = recentProjects[i];
                    recentProjectItem.Click += recentProjectItem_Click;
                    recentProjectsToolStripMenuItem.DropDownItems.Add(recentProjectItem);
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }
        private async void recentProjectItem_Click(object sender, EventArgs e)
        {
            try
            {
                string recentProjectFilePath = (string)((ToolStripMenuItem)sender).Tag;
                if (File.Exists(recentProjectFilePath))
                {
                    await OpenProjectOrSolution(recentProjectFilePath);
                }
                else
                {
                    recentProjectsToolStripMenuItem.DropDownItems.Remove((ToolStripMenuItem)sender);
                    Configuration.Instance.RemoveRecentProjectPath(recentProjectFilePath);
                    Configuration.Instance.Save();
                    throw new Exception("project file '" + recentProjectFilePath + "' does not exist");
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }
        private void MainForm_Resize(object sender, EventArgs e)
        {
            try
            {
                if (alterConfig)
                {
                    Configuration.Instance.WindowState = WindowState == FormWindowState.Maximized || WindowState == FormWindowState.Normal ? WindowState.ToString() : "Normal";
                    if (WindowState == FormWindowState.Normal)
                    {
                        Configuration.Instance.FormSizeX = Width;
                        Configuration.Instance.FormSizeY = Height;
                    }
                    Configuration.Instance.Save();
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }
        private void MainForm_Move(object sender, EventArgs e)
        {
            try
            {
                if (alterConfig)
                {
                    Configuration.Instance.FormTop = Math.Max(0, Top);
                    Configuration.Instance.FormLeft = Math.Max(0, Left);
                    Configuration.Instance.Save();
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }
        private void infoTimer_Tick(object sender, EventArgs e)
        {
            infoTimer.Stop();
            infoLabel.Text = "";
        }
        private void splitter1_SplitterMoved(object sender, SplitterEventArgs e)
        {
            try
            {
                if (alterConfig)
                {
                    Configuration.Instance.VerticalSplitterPos = splitter1.SplitPosition;
                    Configuration.Instance.Save();
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }
        private void editorSplitContainer_SplitterMoved(object sender, SplitterEventArgs e)
        {
            try
            {
                if (alterConfig)
                {
                    Configuration.Instance.HorizontalSplitterPos = editorSplitContainer.SplitterDistance;
                    Configuration.Instance.Save();
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }
        private void openToolStripMenuItem_Click(object sender, EventArgs e)
        {
            try
            {
                SourceFile sourceFile = (SourceFile)solutionExplorerTreeView.SelectedNode.Tag;
                EditSourceFile(sourceFile);
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }

        }
        private Editor EditSourceFile(SourceFile sourceFile)
        {
            fileLabel.Text = sourceFile.FilePath;
            foreach (TabPage editorTabPage in editorTabControl.TabPages)
            {
                Editor currentEditor = (Editor)editorTabPage.Tag;
                if (currentEditor.SourceFile == sourceFile)
                {
                    editorTabControl.SelectedTab = editorTabPage;
                    currentEditor.Mode = EditorMode.editing;
                    currentEditor.Focus();
                    return currentEditor;
                }
            }
            TabPage editorTab = new TabPage(sourceFile.Name);
            Editor editor = new Editor(sourceFile, this);
            editor.Mode = EditorMode.editing;
            editor.TabSize = Configuration.Instance.TabSize;
            editorTab.Tag = editor;
            editor.PositionChanged += editor_PositionChanged;
            editor.DirtyStatusChanged += editor_DirtyStatusChanged;
            editor.Closed += editor_Closed;
            editorTab.Controls.Add(editor);
            editorTabControl.TabPages.Add(editorTab);
            editorTabControl.SelectedTab = editorTab;
            SetMenuItemStatus();
            editor.Focus();
            return editor;
        }
        void editor_PositionChanged(object sender, PositionEventArgs e)
        {
            lineLabel.Text = "Line " + e.Line.ToString();
            colLabel.Text = "Col " + e.Col.ToString();
        }
        void editor_DirtyStatusChanged(object sender, EventArgs e)
        {
            Editor editor = (Editor)sender;
            editorTabControl.SelectedTab.Text = editor.SourceFile.Name + (editor.Dirty ? "*" : "");
        }
        void editor_Closed(object sender, EventArgs e)
        {
            fileLabel.Text = "";
            lineLabel.Text = "";
            colLabel.Text = "";
        }
        private void solutionExplorerTreeView_AfterLabelEdit(object sender, NodeLabelEditEventArgs e)
        {
            try
            {
                if (e.Node.Level == 0)  // renaming a solution node
                {
                    e.CancelEdit = true;
                    if (!string.IsNullOrEmpty(e.Label))
                    {
                        solution.Rename(e.Label);
                        solution.Save();
                        SetSolutionNodeText();
                    }
                }
                else if (e.Node.Level == 1) // renaming a project node
                {
                    if (!string.IsNullOrEmpty(e.Label))
                    {
                        string projectName = e.Node.Text;
                        Project project = solution.GetProject(projectName);
                        if (project != null)
                        {
                            try
                            {
                                solution.RenameProject(project, e.Label);
                            }
                            catch (Exception ex)
                            {
                                e.CancelEdit = true;
                                throw;
                            }
                            solution.Save();
                        }
                        else
                        {
                            e.CancelEdit = true;
                        }
                    }
                    else
                    {
                        e.CancelEdit = true;
                    }
                }
                else if (e.Node.Level == 2) // renaming a source file node
                {
                    if (!string.IsNullOrEmpty(e.Label))
                    {
                        if (Path.GetExtension(e.Label) != ".cm")
                        {
                            e.CancelEdit = true;
                            throw new Exception("'.cm' extension must be supplied");
                        }
                        Project project = solution.GetProject(e.Node.Parent.Text);
                        if (project != null)
                        {
                            SourceFile file = project.GetSourceFile(e.Node.Text);
                            if (file != null)
                            {
                                file.Rename(e.Label);
                                foreach (TabPage editorTab in editorTabControl.TabPages)
                                {
                                    Editor editor = (Editor)editorTab.Tag;
                                    if (editor.SourceFile == file)
                                    {
                                        editorTab.Text = file.Name;
                                        if (editorTab == editorTabControl.SelectedTab)
                                        {
                                            fileLabel.Text = file.FilePath;
                                        }
                                    }
                                }
                                project.Save();
                            }
                            else
                            {
                                e.CancelEdit = true;
                            }
                        }
                    }
                    else
                    {
                        e.CancelEdit = true;
                    }
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message, "Cmajor Development Environment");
            }
        }
        private void SetSolutionNodeText()
        {
            int n = solution.Projects.Count;
            solutionExplorerTreeView.Nodes[0].Text = "Solution '" + solution.Name + "' (" + n.ToString() + " project" + (n == 1 ? ")" : "s)");
        }
        private void solutionExplorerTreeView_NodeMouseClick(object sender, TreeNodeMouseClickEventArgs e)
        {
            try
            {
                solutionExplorerTreeView.SelectedNode = e.Node;
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }
        private void solutionExplorerTreeView_NodeMouseDoubleClick(object sender, TreeNodeMouseClickEventArgs e)
        {
            try
            {
                if (e.Node.Level == 2)
                {
                    SourceFile sourceFile = e.Node.Tag as SourceFile;
                    if (sourceFile != null)
                    {
                        EditSourceFile(sourceFile);
                    }
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }
        private void UneditSourceFile(SourceFile sourceFile)
        {
            foreach (TabPage editorTabPage in editorTabControl.TabPages)
            {
                Editor currentEditor = (Editor)editorTabPage.Tag;
                if (currentEditor.SourceFile == sourceFile)
                {
                    editorTabControl.TabPages.Remove(editorTabPage);
                    return;
                }
            }
        }
        private async void removeToolStripMenuItem1_Click(object sender, EventArgs e)
        {
            try
            {
                TreeNode sourceFileNode = solutionExplorerTreeView.SelectedNode;
                SourceFile sourceFile = (SourceFile)sourceFileNode.Tag;
                if (MessageBox.Show("'" + sourceFile.Name + "' will be removed.", "Cmajor Development Environment", MessageBoxButtons.OKCancel) == DialogResult.OK)
                {
                    UneditSourceFile(sourceFile);
                    TreeNode projectNode = sourceFileNode.Parent;
                    Project project = (Project)projectNode.Tag;
                    project.SourceFiles.Remove(sourceFile);
                    project.Save();
                    await SetupSolutionExplorer(project);
                    SetMenuItemStatus();
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }
        private async void renameToolStripMenuItem2_Click(object sender, EventArgs e)
        {
            solutionExplorerTreeView.SelectedNode.BeginEdit();
        }
        private async void deleteToolStripMenuItem_Click(object sender, EventArgs e)
        {
            try
            {
                TreeNode sourceFileNode = solutionExplorerTreeView.SelectedNode;
                SourceFile sourceFile = (SourceFile)sourceFileNode.Tag;
                if (MessageBox.Show("'" + sourceFile.FilePath + "' will be deleted.", "Cmajor Development Environment", MessageBoxButtons.OKCancel) == DialogResult.OK)
                {
                    UneditSourceFile(sourceFile);
                    TreeNode projectNode = sourceFileNode.Parent;
                    Project project = (Project)projectNode.Tag;
                    project.SourceFiles.Remove(sourceFile);
                    project.Save();
                    File.Delete(sourceFile.FilePath);
                    await SetupSolutionExplorer(project);
                    SetMenuItemStatus();
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }
        private async void closeSolutionToolStripMenuItem_Click(object sender, EventArgs e)
        {
            try
            {
                if (solution != null)
                {
                    await CloseSolution();
                }
                SetMenuItemStatus();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }
        private void WriteOutputLine(string line)
        {
            outputRichTextBox.AppendText(line + Environment.NewLine);
            outputRichTextBox.ScrollToCaret();
        }
        private void WriteToConsole(string chars)
        {
            console.Write(chars);
        }
        private void ProjectRun()
        {
            abortToolStripMenuItem.Enabled = false;
            processRunning = false;
            console.ReadOnly = true;
            SetState(State.editing);
        }
        private void exitToolStripMenuItem_Click(object sender, EventArgs e)
        {
            try
            {
                Configuration.Instance.Save();
                Application.Exit();
                SetMenuItemStatus();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }
        private void MainForm_FormClosing(object sender, FormClosingEventArgs e)
        {
            try
            {
                foreach (TabPage editorTab in editorTabControl.TabPages)
                {
                    Editor editor = (Editor)editorTab.Tag;
                    DialogResult result = editor.Close();
                    if (result == DialogResult.Cancel)
                    {
                        e.Cancel = true;
                        return;
                    }
                }
                compiler.DoExit();
                compiler.WaitForExit();
                executor.DoExit();
                executor.WaitForExit();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }
        private void SaveAll()
        {
            try
            {
                if (solution != null)
                {
                    solution.Save();
                    foreach (TabPage editorTab in editorTabControl.TabPages)
                    {
                        Editor editor = (Editor)editorTab.Tag;
                        editor.Save();
                    }
                }
                Configuration.Instance.Save();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }
        private async void newProjectSolutionToolStripMenuItem_Click(object sender, EventArgs e)
        {
            try
            {
                NewProjectDialog dialog = new NewProjectDialog(true, solution != null);
                string cmajorProjectsDir = Environment.GetEnvironmentVariable("CMAJOR_ROOT") + "\\projects";
                if (!Directory.Exists(cmajorProjectsDir))
                {
                    Directory.CreateDirectory(cmajorProjectsDir);
                }
                dialog.ProjectLocation = solution != null ? solution.BasePath : cmajorProjectsDir;
                DialogResult result = dialog.ShowDialog();
                if (result == DialogResult.OK)
                {
                    if (dialog.NewSolution || dialog.SelectedProjectType == ProjectType.blankSolution)
                    {
                        if (solution != null)
                        {
                            if (!await CloseSolution())
                            {
                                return;
                            }
                        }
                    }
                    if (dialog.SelectedProjectType == ProjectType.blankSolution)
                    {
                        string solutionName = Path.GetFileNameWithoutExtension(dialog.ProjectName);
                        string solutionFilePath = Path.Combine(Path.Combine(dialog.ProjectLocation, solutionName), solutionName + ".cms");
                        Directory.CreateDirectory(Path.GetDirectoryName(solutionFilePath));
                        solution = new Solution(solutionName, solutionFilePath);
                        await SetupSolutionExplorer(null);
                        Configuration.Instance.AddRecentProject(solutionFilePath);
                        Configuration.Instance.Save();
                        SetupRecentProjectMenu();
                    }
                    else
                    {
                        string projectName = dialog.ProjectName;
                        if (projectName.EndsWith(".cmp") || projectName.EndsWith(".cms"))
                        {
                            projectName = projectName.Remove(projectName.Length - 4);
                        }
                        string solutionDirectory = Path.Combine(dialog.ProjectLocation, projectName);
                        Directory.CreateDirectory(solutionDirectory);
                        if (dialog.NewSolution)
                        {
                            string solutionName = projectName;
                            string solutionFilePath = Path.Combine(solutionDirectory, solutionName + ".cms");
                            solution = new Solution(solutionName, solutionFilePath);
                            await SetupSolutionExplorer(null);
                            Configuration.Instance.AddRecentProject(solutionFilePath);
                            Configuration.Instance.Save();
                            SetupRecentProjectMenu();
                        }
                        Project project = new Project(projectName, Path.Combine(solutionDirectory, projectName + ".cmp"));
                        if (dialog.SelectedProjectType == ProjectType.consoleApp)
                        {
                            project.Target = Target.program;
                        }
                        else if (dialog.SelectedProjectType == ProjectType.library)
                        {
                            project.Target = Target.library;
                        }
                        solution.AddProject(project);
                        TreeNode solutionNode = solutionExplorerTreeView.Nodes[0];
                        TreeNode projectNode = solutionNode.Nodes.Add(project.Name);
                        projectNode.Tag = project;
                        projectNode.ContextMenuStrip = projectContextMenuStrip;
                    }
                    await SetupSolutionExplorer(null);
                    solution.Save();
                }
                SetMenuItemStatus();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }
        private async void newSourceFileToolStripMenuItem_Click(object sender, EventArgs e)
        {
            try
            {
                Project project = (Project)solutionExplorerTreeView.SelectedNode.Tag;
                NewSourceFileDialog dialog = new NewSourceFileDialog();
                dialog.SourceFileName = "file1.cm";
                if (dialog.ShowDialog() == DialogResult.OK)
                {
                    string sourceFileName = dialog.SourceFileName;
                    SourceFile.Kind kind = SourceFile.Kind.cm;
                    if (!sourceFileName.EndsWith(".cm"))
                    {
                        kind = SourceFile.Kind.text;
                    }
                    SourceFile sourceFile = project.AddSourceFile(sourceFileName, kind);
                    using (StreamWriter writer = File.CreateText(sourceFile.FilePath))
                    {
                    }
                    solution.Save();
                    await SetupSolutionExplorer(project);
                    SetMenuItemStatus();
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }
        private void buildSolutionToolStripMenuItem_Click(object sender, EventArgs e)
        {
            BuildSolution();
        }
        private void buildToolStripMenuItem1_Click(object sender, EventArgs e)
        {
            BuildSolution();
        }
        private void buildActiveProjectToolStripMenuItem_Click(object sender, EventArgs e)
        {
            BuildProject(solution.ActiveProject);
        }
        private void buildToolStripMenuItem2_Click(object sender, EventArgs e)
        {
            TreeNode selectedNode = solutionExplorerTreeView.SelectedNode;
            if (selectedNode != null)
            {
                Project selectedProject = selectedNode.Tag as Project;
                if (selectedProject != null)
                {
                    BuildProject(selectedProject);
                }
            }
        }
        private void BuildSolution()
        {
            try
            {
                if (processRunning)
                {
                    throw new Exception("Cannot build while process is running");
                }
                if (state == State.debugging)
                {
                    throw new Exception("Stop debugging before building");
                }
                SaveAll();
                progressTimer.Start();
                outputRichTextBox.Clear();
                errorListView.Items.Clear();
                showErrorDescriptionInTextWindowToolStripMenuItem.Enabled = false;
                buildSolutionToolStripMenuItem.Enabled = false;
                buildToolStripMenuItem1.Enabled = false;
                buildActiveProjectToolStripMenuItem.Enabled = false;
                outputTabControl.SelectedTab = outputTabPage;
                if (solution != null && solution.Projects.Count > 0)
                {
                    string config = configComboBox.Text;
                    compileStartTime = DateTime.Now;
                    compileTimer.Start();
                    cancelToolStripMenuItem.Enabled = true;
                    buildInProgress = true;
                    SetState(State.compiling);
                    compiler.DoCompile(solution.FilePath, config, emitLlvm, emitOptLlvm, optimizationLevel);
                    infoLabel.Text = "Building";
                }
            }
            catch (Exception ex)
            {
                SetState(State.editing);
                MessageBox.Show(ex.Message);
            }
        }
        private void BuildProject(Project project)
        {
            try
            {
                if (processRunning)
                {
                    throw new Exception("Cannot build while process is running");
                }
                SaveAll();
                progressTimer.Start();
                outputRichTextBox.Clear();
                errorListView.Items.Clear();
                showErrorDescriptionInTextWindowToolStripMenuItem.Enabled = false;
                buildSolutionToolStripMenuItem.Enabled = false;
                buildToolStripMenuItem1.Enabled = false;
                buildActiveProjectToolStripMenuItem.Enabled = false;
                outputTabControl.SelectedTab = outputTabPage;
                string config = configComboBox.Text;
                compileStartTime = DateTime.Now;
                compileTimer.Start();
                cancelToolStripMenuItem.Enabled = true;
                buildInProgress = true;
                SetState(State.compiling);
                compiler.DoCompile(project.FilePath, config, emitLlvm, emitOptLlvm, optimizationLevel);
                infoLabel.Text = "Building";
            }
            catch (Exception ex)
            {
                SetState(State.editing);
                MessageBox.Show(ex.Message);
            }
        }
        private void HandleCompileResult(CompileResult compileResult)
        {
            cancelToolStripMenuItem.Enabled = false;
            buildInProgress = false;
            compileTimer.Stop();
            compileTimeStatusLabel.Text = "";
            progressTimer.Stop();
            progressLabel.Text = "";
            bool hasErrors = !compileResult.Success;
            bool hasWarningsOrInfos = false;
            if (hasErrors || hasWarningsOrInfos)
            {
                if (hasErrors)
                {
                    if (compiling)
                    {
                        compiling = false;
                        infoLabel.Text = "Compile failed";
                    }
                    else if (cleaning)
                    {
                        cleaning = false;
                        infoLabel.Text = "Clean failed";
                    }
                    else
                    {
                        infoLabel.Text = "Build failed";
                    }
                }
                else
                {
                    if (compileAborted)
                    {
                        compileAborted = false;
                        infoLabel.Text = "Compile aborted";
                    }
                    else if (compiling)
                    {
                        compiling = false;
                        infoLabel.Text = "Compile succeeded";
                    }
                    else if (cleaning)
                    {
                        cleaning = false;
                        infoLabel.Text = "Clean succeeded";
                    }
                    else
                    {
                        infoLabel.Text = "Build succeeded";
                    }
                }
                Diagnostics diagnostics = compileResult.Diagnostics;
                if (diagnostics != null)
                {
                    string tool = diagnostics.Tool;
                    string file = "";
                    string line = "";
                    string text = "";
                    Reference mainReference = null;
                    if (diagnostics.References.Count > 0)
                    {
                        mainReference = diagnostics.References[0];
                        file = mainReference.File;
                        if (mainReference.Line != 0)
                        {
                            line = mainReference.Line.ToString();
                        }
                        else
                        {
                            line = "";
                        }
                        text = mainReference.Text.Trim();
                    }
                    ListViewItem item = new ListViewItem(new string[] { tool, diagnostics.Kind, diagnostics.Message, file, line, diagnostics.Project, text });
                    item.Tag = mainReference;
                    errorListView.Items.Add(item);
                    for (int i = 1; i < diagnostics.References.Count; ++i)
                    {
                        Reference reference = diagnostics.References[i];
                        file = reference.File;
                        if (reference.Line != 0)
                        {
                            line = reference.Line.ToString();
                        }
                        else
                        {
                            line = "";
                        }
                        text = reference.Text.Trim();
                        ListViewItem refItem = new ListViewItem(new string[] { "cmc", "info", "see reference to", file, line, "", text });
                        refItem.Tag = reference;
                        errorListView.Items.Add(refItem);
                    }
                    showErrorDescriptionInTextWindowToolStripMenuItem.Enabled = true;
                }
                outputTabControl.SelectedTab = errorTabPage;
            }
            else
            {
                if (compileAborted)
                {
                    compileAborted = false;
                    infoLabel.Text = "Compile aborted";
                }
                else if (compiling)
                {
                    compiling = false;
                    infoLabel.Text = "Compile succeeded";
                }
                else if (cleaning)
                {
                    cleaning = false;
                    infoLabel.Text = "Clean succeeded";
                }
                else
                {
                    infoLabel.Text = "Build succeeded";
                }
            }
            infoTimer.Start();
            SetState(State.editing);
        }
        private void showErrorDescriptionInTextWindowToolStripMenuItem_Click(object sender, EventArgs e)
        {
            try
            {
                if (errorListView.SelectedItems.Count > 0)
                {
                    ListViewItem item = errorListView.SelectedItems[0];
                    string description = item.SubItems[2].Text;
                    TextVisualizerForm form = new TextVisualizerForm();
                    form.Text = "Error Description";
                    form.Left = Configuration.Instance.TextVisX;
                    form.Top = Configuration.Instance.TextVisY;
                    form.Width = Configuration.Instance.TextVisW;
                    form.Height = Configuration.Instance.TextVisH;
                    form.TextContent = description;
                    form.ShowDialog();
                    Configuration.Instance.TextVisX = form.Left;
                    Configuration.Instance.TextVisY = form.Top;
                    Configuration.Instance.TextVisW = form.Width;
                    Configuration.Instance.TextVisH = form.Height;
                    Configuration.Instance.Save();
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }
        private void errorListView_ItemActivate(object sender, EventArgs e)
        {
            try
            {
                if (errorListView.SelectedItems.Count > 0)
                {
                    ListViewItem selectedItem = errorListView.SelectedItems[0];
                    string projectName = selectedItem.SubItems[5].Text;
                    Reference reference = (Reference)selectedItem.Tag;
                    if (reference != null)
                    {
                        string filePath = reference.File;
                        if (!string.IsNullOrEmpty(filePath))
                        {
                            SourceFile sourceFile = solution.GetSourceFileByPath(filePath);
                            if (sourceFile == null)
                            {
                                sourceFile = new SourceFile(Path.GetFileName(filePath), filePath, (filePath.EndsWith(".cm") ? SourceFile.Kind.cm : SourceFile.Kind.text));
                            }
                            Editor editor = EditSourceFile(sourceFile);
                            int line = reference.Line;
                            if (line != 0)
                            {
                                int startCol = reference.StartCol;
                                int endCol = reference.EndCol;
                                if (startCol != 0 && endCol == 0 || startCol == endCol)
                                {
                                    endCol = startCol + 1;
                                }
                                LineCol startLineCol = new LineCol(line, startCol);
                                LineCol endLineCol = new LineCol(line, endCol);
                                editor.SetCursorPos(line, 1);
                                editor.SetHilite(startLineCol, endLineCol, Editor.Hilite.errorPos);
                            }
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }
        private void progressTimer_Tick(object sender, EventArgs e)
        {
            progressLabel.Text = new string(progressChars[progressIndex], 1);
            progressIndex = (progressIndex + 1) % progressChars.Length;
        }
        private void errorListView_ColumnWidthChanged(object sender, ColumnWidthChangedEventArgs e)
        {
            try
            {
                switch (e.ColumnIndex)
                {
                    case 0: Configuration.Instance.ToolColWidth = errorListView.Columns[0].Width; break;
                    case 1: Configuration.Instance.CategoryColWidth = errorListView.Columns[1].Width; break;
                    case 2: Configuration.Instance.DescriptionColWidth = errorListView.Columns[2].Width; break;
                    case 3: Configuration.Instance.FileColWidth = errorListView.Columns[3].Width; break;
                    case 4: Configuration.Instance.LineColWidth = errorListView.Columns[4].Width; break;
                    case 5: Configuration.Instance.ProjectColWidth = errorListView.Columns[5].Width; break;
                    case 6: Configuration.Instance.TextColWidth = errorListView.Columns[6].Width; break;
                }
                Configuration.Instance.Save();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }
        private void saveAllToolStripMenuItem_Click(object sender, EventArgs e)
        {
            try
            {
                SaveAll();
                SetMenuItemStatus();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }
        private void saveToolStripMenuItem_Click(object sender, EventArgs e)
        {
            try
            {
                TabPage editorTab = editorTabControl.SelectedTab;
                if (editorTab != null)
                {
                    Editor editor = (Editor)editorTab.Tag;
                    editor.Save();
                }
                SetMenuItemStatus();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }
        private void closeToolStripMenuItem_Click(object sender, EventArgs e)
        {
            try
            {
                TabPage editorTab = editorTabControl.SelectedTab;
                if (editorTab != null)
                {
                    Editor editor = (Editor)editorTab.Tag;
                    DialogResult result = editor.Close();
                    if (result == DialogResult.Cancel)
                    {
                        return;
                    }
                    editorTabControl.TabPages.RemoveAt(editorTabControl.SelectedIndex);
                }
                SetMenuItemStatus();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }
        private void optionsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            try
            {
                BuildOptionsDialog dialog = new BuildOptionsDialog();
                emitLlvm = Configuration.Instance.EmitLlvm;
                emitOptLlvm = Configuration.Instance.EmitOptLlvm;
                dialog.EmitLlvm = emitLlvm;
                dialog.EmitOptLlvm = emitOptLlvm;
                if (dialog.ShowDialog() == System.Windows.Forms.DialogResult.OK)
                {
                    emitLlvm = dialog.EmitLlvm;
                    emitOptLlvm = dialog.EmitOptLlvm;
                    Configuration.Instance.EmitLlvm = emitLlvm;
                    Configuration.Instance.EmitOptLlvm = emitOptLlvm;
                    Configuration.Instance.Save();
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }
        private void intermediateCodeToolStripMenuItem_Click(object sender, EventArgs e)
        {
            try
            {
                SourceFile sourceFile = (SourceFile)solutionExplorerTreeView.SelectedNode.Tag;
                string intermediateCodePath = sourceFile.GetIntermediateCodePath(configComboBox.Text);
                EditSourceFile(new SourceFile(Path.GetFileName(intermediateCodePath), intermediateCodePath, SourceFile.Kind.text));
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }
        private void optimizedIntermediateCodeToolStripMenuItem_Click(object sender, EventArgs e)
        {
            try
            {
                SourceFile sourceFile = (SourceFile)solutionExplorerTreeView.SelectedNode.Tag;
                string optimizedIntermediateCodePath = sourceFile.GetOptimizedIntermediateCodePath(configComboBox.Text);
                EditSourceFile(new SourceFile(Path.GetFileName(optimizedIntermediateCodePath), optimizedIntermediateCodePath, SourceFile.Kind.text));
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }
        private void viewFileToolStripMenuItem_DropDownOpened(object sender, EventArgs e)
        {
            try
            {
                SourceFile sourceFile = (SourceFile)solutionExplorerTreeView.SelectedNode.Tag;
                string intermediateCodePath = sourceFile.GetIntermediateCodePath(configComboBox.Text);
                intermediateCodeToolStripMenuItem.Enabled = File.Exists(intermediateCodePath);
                string optimizedIntermediateCodePath = sourceFile.GetOptimizedIntermediateCodePath(configComboBox.Text);
                optimizedIntermediateCodeToolStripMenuItem.Enabled = File.Exists(optimizedIntermediateCodePath);
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }
        private void cleanSolutionToolStripMenuItem_Click(object sender, EventArgs e)
        {
            CleanProjectOrSolution(solution.FilePath);
        }
        private void cleanSolutionToolStripMenuItem2_Click(object sender, EventArgs e)
        {
            CleanProjectOrSolution(solution.FilePath);
        }
        private void CleanProjectOrSolution(string solutionOrProjectFilePath)
        {
            try
            {
                if (state == State.debugging)
                {
                    throw new Exception("Stop debugging before cleaning");
                }
                string config = configComboBox.Text;
                SaveAll();
                progressTimer.Start();
                outputRichTextBox.Clear();
                errorListView.Items.Clear();
                showErrorDescriptionInTextWindowToolStripMenuItem.Enabled = false;
                buildSolutionToolStripMenuItem.Enabled = false;
                buildToolStripMenuItem1.Enabled = false;
                buildActiveProjectToolStripMenuItem.Enabled = false;
                outputTabControl.SelectedTab = outputTabPage;
                cleaning = true;
                cancelToolStripMenuItem.Enabled = true;
                buildInProgress = true;
                SetState(State.compiling);
                compiler.DoClean(solutionOrProjectFilePath, config);
                infoLabel.Text = "Cleaning";
            }
            catch (Exception ex)
            {
                SetState(State.editing);
                MessageBox.Show(ex.Message);
            }
        }
        private void cleanActiveProjectToolStripMenuItem_Click(object sender, EventArgs e)
        {
            CleanProjectOrSolution(solution.ActiveProject.FilePath);
        }
        private void cleanProjectToolStripMenuItem_Click(object sender, EventArgs e)
        {
            TreeNode selectedNode = solutionExplorerTreeView.SelectedNode;
            if (selectedNode != null)
            {
                Project selectedProject = selectedNode.Tag as Project;
                if (selectedProject != null)
                {
                    CleanProjectOrSolution(selectedProject.FilePath);
                }
            }
        }
        private void closeAllToolStripMenuItem_Click(object sender, EventArgs e)
        {
            try
            {
                foreach (TabPage editorTab in editorTabControl.TabPages)
                {
                    Editor editor = (Editor)editorTab.Tag;
                    DialogResult result = editor.Close();
                    if (result == DialogResult.Cancel)
                    {
                        break;
                    }
                    editorTabControl.TabPages.Remove(editorTab);
                }
                SetMenuItemStatus();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }
        private async void existingSourceFileToolStripMenuItem_Click(object sender, EventArgs e)
        {
            try
            {
                Project project = (Project)solutionExplorerTreeView.SelectedNode.Tag;
                addExistingSourceFileDialog.InitialDirectory = project.BasePath;
                if (addExistingSourceFileDialog.ShowDialog() == DialogResult.OK)
                {
                    string fileName = addExistingSourceFileDialog.FileName;
                    SourceFile.Kind kind = SourceFile.Kind.cm;
                    if (!fileName.EndsWith(".cm"))
                    {
                        kind = SourceFile.Kind.text;
                    }
                    project.AddSourceFile(fileName, kind);
                    solution.Save();
                    await SetupSolutionExplorer(project);
                    SetMenuItemStatus();
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }
        private async void newProjectToolStripMenuItem1_Click(object sender, EventArgs e)
        {
            try
            {
                NewProjectDialog dialog = new NewProjectDialog(false, solution != null);
                dialog.ProjectLocation = solution.BasePath;
                DialogResult result = dialog.ShowDialog();
                if (result == DialogResult.OK)
                {
                    string projectName = dialog.ProjectName;
                    if (projectName.EndsWith(".cmp"))
                    {
                        projectName = projectName.Remove(projectName.Length - 4);
                    }
                    Project project = new Project(projectName, Path.Combine(Path.Combine(dialog.ProjectLocation, projectName), projectName + ".cmp"));
                    Directory.CreateDirectory(Path.GetDirectoryName(project.FilePath));
                    if (dialog.SelectedProjectType == ProjectType.consoleApp)
                    {
                        project.Target = Target.program;
                    }
                    else if (dialog.SelectedProjectType == ProjectType.library)
                    {
                        project.Target = Target.library;
                    }
                    solution.AddProject(project);
                    TreeNode solutionNode = solutionExplorerTreeView.Nodes[0];
                    TreeNode projectNode = solutionNode.Nodes.Add(project.Name);
                    projectNode.Tag = project;
                    projectNode.ContextMenuStrip = projectContextMenuStrip;
                    solution.Save();
                    await SetupSolutionExplorer(null);
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }
        private async void existingProjectToolStripMenuItem_Click(object sender, EventArgs e)
        {
            try
            {
                addExistingProjectFileDialog.InitialDirectory = solution.BasePath;
                if (addExistingProjectFileDialog.ShowDialog() == DialogResult.OK)
                {
                    ProjectFile projectFileParser = ParserRepository.Instance.ProjectFileParser;
                    Project project = projectFileParser.Parse(File.ReadAllText(addExistingProjectFileDialog.FileName), 0, addExistingProjectFileDialog.FileName);
                    solution.AddProject(project);
                    TreeNode solutionNode = solutionExplorerTreeView.Nodes[0];
                    TreeNode projectNode = solutionNode.Nodes.Add(project.Name);
                    projectNode.Tag = project;
                    projectNode.ContextMenuStrip = projectContextMenuStrip;
                    solution.Save();
                    await SetupSolutionExplorer(null);
                    SetMenuItemStatus();
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }
        private void renameToolStripMenuItem1_Click(object sender, EventArgs e)
        {
            solutionExplorerTreeView.SelectedNode.BeginEdit();
        }
        private async void removeToolStripMenuItem_Click(object sender, EventArgs e)
        {
            try
            {
                TreeNode node = solutionExplorerTreeView.SelectedNode;
                Project toRemove = (Project)node.Tag;
                if (MessageBox.Show("'" + toRemove.Name + "' will be removed.", "Cmajor Development Environment", MessageBoxButtons.OKCancel) == DialogResult.OK)
                {
                    solution.RemoveProject(toRemove);
                    solution.Save();
                    await SetupSolutionExplorer(null);
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }
        private void moduleContentToolStripMenuItem_Click(object sender, EventArgs e)
        {
            try
            {
                TreeNode selectedNode = solutionExplorerTreeView.SelectedNode;
                if (selectedNode != null)
                {
                    Project selectedProject = selectedNode.Tag as Project;
                    if (selectedProject != null)
                    {
                        string moduleFilePath = selectedProject.GetModuleFilePath(configComboBox.Text);
                        System.Diagnostics.ProcessStartInfo startInfo = new System.Diagnostics.ProcessStartInfo("cmmdump.exe", moduleFilePath);
                        startInfo.UseShellExecute = false;
                        startInfo.CreateNoWindow = true;
                        startInfo.RedirectStandardOutput = true;
                        startInfo.RedirectStandardError = true;
                        System.Diagnostics.Process cmlDump = System.Diagnostics.Process.Start(startInfo);
                        string output = cmlDump.StandardOutput.ReadToEnd();
                        string errorText = cmlDump.StandardError.ReadToEnd();
                        cmlDump.WaitForExit();
                        if (cmlDump.ExitCode != 0)
                        {
                            throw new Exception(errorText);
                        }
                        else
                        {
                            string cmmTextFilePath = moduleFilePath + ".txt";
                            using (StreamWriter writer = File.CreateText(cmmTextFilePath))
                            {
                                writer.Write(output);
                            }
                            EditSourceFile(new SourceFile(selectedProject.Name + ".cmm", cmmTextFilePath, SourceFile.Kind.text));
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }
        private async void newTextFileToolStripMenuItem_Click(object sender, EventArgs e)
        {
            try
            {
                Project project = (Project)solutionExplorerTreeView.SelectedNode.Tag;
                NewTextFileDialog dialog = new NewTextFileDialog();
                dialog.TextFileName = "file1.txt";
                if (dialog.ShowDialog() == DialogResult.OK)
                {
                    string textFileName = dialog.TextFileName;
                    SourceFile.Kind kind = SourceFile.Kind.text;
                    SourceFile sourceFile = project.AddSourceFile(textFileName, kind);
                    using (StreamWriter writer = File.CreateText(sourceFile.FilePath))
                    {
                    }
                    solution.Save();
                    await SetupSolutionExplorer(project);
                    SetMenuItemStatus();
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }
        private void clearRecentProjectsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            try
            {
                Configuration.Instance.ClearRecentProjects();
                SetupRecentProjectMenu();
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }
        private async void systemLibraryToolStripMenuItem_Click(object sender, EventArgs e)
        {
            try
            {
                string cmajorRootDir = Environment.GetEnvironmentVariable("CMAJOR_ROOT");
                await OpenProjectOrSolution(Path.Combine(Path.Combine(cmajorRootDir, "system"), "System.cms")); ;
            }
            catch (Exception ex)
            {
                MessageBox.Show(ex.Message);
            }
        }
        private Solution solution;
        private XTabControl editorTabControl;
        private State state;
        private bool alterConfig;
        private Compiler compiler;
        private Executor executor;
        private ConsoleWindow console;
        private bool buildInProgress;
        private string progressChars;
        private int progressIndex;
        private bool processRunning;
        private bool compileAborted;
        private bool compiling;
        private bool cleaning;
        private bool emitLlvm;
        private bool emitOptLlvm;
        private int optimizationLevel;
        private DateTime compileStartTime;
    }
}
