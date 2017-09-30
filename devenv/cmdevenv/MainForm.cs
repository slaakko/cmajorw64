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
            editorTabControl = new XTabControl();
            editorTabControl.Dock = System.Windows.Forms.DockStyle.Fill;
            editorTabControl.Location = new System.Drawing.Point(0, 0);
            editorTabControl.Name = "editorTabControl";
            editorTabControl.SelectedIndex = 0;
            editorTabControl.Size = new System.Drawing.Size(400, 325);
            editorTabControl.TabIndex = 0;
            editorTabControl.TabClosing += editorTabControl_TabClosing;
            editorSplitContainer.Panel1.Controls.Add(editorTabControl);
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
                    SolutionFile solutionFileParser = ParserRepository.Instance.SolutionFileParser;
                    solution = solutionFileParser.Parse(File.ReadAllText(solutionFileName), 0, solutionFileName);
                }
                else if (projectOrSolutionName.EndsWith(".cmp"))
                {
                    ProjectFile projectFileParser = ParserRepository.Instance.ProjectFileParser;
                    Project project = projectFileParser.Parse(File.ReadAllText(projectOrSolutionName), 0, projectOrSolutionName);
                    solution = new Solution(project.Name, Path.ChangeExtension(project.FilePath, ".cms"));
                    solution.Projects.Add(project);
                    solution.Save();
                    Configuration.Instance.AddRecentProject(solution.FilePath);
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
            rebuildSolutionToolStripMenuItem.Enabled = editing && solutionHasProjects;
            buildActiveProjectToolStripMenuItem.Enabled = editing && activeProjectSet;
            rebuildActiveProjectToolStripMenuItem.Enabled = editing && activeProjectSet;
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
            compileCurrentFileToolStripMenuItem.Enabled = false;
            if (editing && editorOpen)
            {
                TabPage selectedTab = editorTabControl.SelectedTab;
                if (selectedTab != null)
                {
                    Editor editor = (Editor)selectedTab.Tag;
                    if (editor.SourceFile.GetKind() == SourceFile.Kind.cm)
                    {
                        compileCurrentFileToolStripMenuItem.Enabled = true;
                    }
                }
            }
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
            foreach (TabPage editorTabPage in editorTabControl.TabPages)
            {
                Editor editor = (Editor)editorTabPage.Tag;
                if (editor != null)
                {
/*
                    if (this.state == State.editing)
                    {
                        editor.Mode = EditorMode.editing;
                    }
                    else if (this.state == State.debugging)
                    {
                        editor.Mode = EditorMode.debugging;
                    }
*/
                }
            }
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
                if (Configuration.Instance.StartColumnColWidth != 0) startColColumnHeader.Width = Configuration.Instance.StartColumnColWidth;
                if (Configuration.Instance.EndColumnColWidth != 0) endColColumnHeader.Width = Configuration.Instance.EndColumnColWidth;
                if (Configuration.Instance.ProjectColWidth != 0) projectColumnHeader.Width = Configuration.Instance.ProjectColWidth;
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
                            solution.RenameProject(project, e.Label);
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
        private Solution solution;
        private XTabControl editorTabControl;
        private State state;
        private bool alterConfig;
    }
}
