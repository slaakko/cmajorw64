namespace cmdevenv
{
    partial class BuildOptionsDialog
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            this.okButton = new System.Windows.Forms.Button();
            this.cancelButton = new System.Windows.Forms.Button();
            this.groupBox1 = new System.Windows.Forms.GroupBox();
            this.emitOptLlvmCheckBox = new System.Windows.Forms.CheckBox();
            this.emitLlvmCheckBox = new System.Windows.Forms.CheckBox();
            this.groupBox2 = new System.Windows.Forms.GroupBox();
            this.linkWithDebugRuntimeCheckBox = new System.Windows.Forms.CheckBox();
            this.groupBox1.SuspendLayout();
            this.groupBox2.SuspendLayout();
            this.SuspendLayout();
            // 
            // okButton
            // 
            this.okButton.Location = new System.Drawing.Point(92, 186);
            this.okButton.Name = "okButton";
            this.okButton.Size = new System.Drawing.Size(75, 23);
            this.okButton.TabIndex = 6;
            this.okButton.Text = "OK";
            this.okButton.UseVisualStyleBackColor = true;
            this.okButton.Click += new System.EventHandler(this.okButton_Click);
            // 
            // cancelButton
            // 
            this.cancelButton.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.cancelButton.Location = new System.Drawing.Point(173, 186);
            this.cancelButton.Name = "cancelButton";
            this.cancelButton.Size = new System.Drawing.Size(75, 23);
            this.cancelButton.TabIndex = 7;
            this.cancelButton.Text = "Cancel";
            this.cancelButton.UseVisualStyleBackColor = true;
            // 
            // groupBox1
            // 
            this.groupBox1.Controls.Add(this.emitOptLlvmCheckBox);
            this.groupBox1.Controls.Add(this.emitLlvmCheckBox);
            this.groupBox1.Location = new System.Drawing.Point(12, 12);
            this.groupBox1.Name = "groupBox1";
            this.groupBox1.Size = new System.Drawing.Size(234, 90);
            this.groupBox1.TabIndex = 12;
            this.groupBox1.TabStop = false;
            this.groupBox1.Text = "LLVM options";
            // 
            // emitOptLlvmCheckBox
            // 
            this.emitOptLlvmCheckBox.AutoSize = true;
            this.emitOptLlvmCheckBox.Location = new System.Drawing.Point(17, 50);
            this.emitOptLlvmCheckBox.Name = "emitOptLlvmCheckBox";
            this.emitOptLlvmCheckBox.Size = new System.Drawing.Size(211, 17);
            this.emitOptLlvmCheckBox.TabIndex = 1;
            this.emitOptLlvmCheckBox.Text = "Emit optimized LLVM intermediate code";
            this.emitOptLlvmCheckBox.UseVisualStyleBackColor = true;
            // 
            // emitLlvmCheckBox
            // 
            this.emitLlvmCheckBox.AutoSize = true;
            this.emitLlvmCheckBox.Location = new System.Drawing.Point(17, 27);
            this.emitLlvmCheckBox.Name = "emitLlvmCheckBox";
            this.emitLlvmCheckBox.Size = new System.Drawing.Size(164, 17);
            this.emitLlvmCheckBox.TabIndex = 0;
            this.emitLlvmCheckBox.Text = "Emit LLVM intermediate code";
            this.emitLlvmCheckBox.UseVisualStyleBackColor = true;
            // 
            // groupBox2
            // 
            this.groupBox2.Controls.Add(this.linkWithDebugRuntimeCheckBox);
            this.groupBox2.Location = new System.Drawing.Point(12, 108);
            this.groupBox2.Name = "groupBox2";
            this.groupBox2.Size = new System.Drawing.Size(234, 61);
            this.groupBox2.TabIndex = 13;
            this.groupBox2.TabStop = false;
            this.groupBox2.Text = "Debugging options";
            // 
            // linkWithDebugRuntimeCheckBox
            // 
            this.linkWithDebugRuntimeCheckBox.AutoSize = true;
            this.linkWithDebugRuntimeCheckBox.Location = new System.Drawing.Point(17, 28);
            this.linkWithDebugRuntimeCheckBox.Name = "linkWithDebugRuntimeCheckBox";
            this.linkWithDebugRuntimeCheckBox.Size = new System.Drawing.Size(138, 17);
            this.linkWithDebugRuntimeCheckBox.TabIndex = 0;
            this.linkWithDebugRuntimeCheckBox.Text = "Link with debug runtime";
            this.linkWithDebugRuntimeCheckBox.UseVisualStyleBackColor = true;
            // 
            // BuildOptionsDialog
            // 
            this.AcceptButton = this.okButton;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.CancelButton = this.cancelButton;
            this.ClientSize = new System.Drawing.Size(260, 228);
            this.Controls.Add(this.groupBox2);
            this.Controls.Add(this.groupBox1);
            this.Controls.Add(this.cancelButton);
            this.Controls.Add(this.okButton);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "BuildOptionsDialog";
            this.Text = "Build Options";
            this.groupBox1.ResumeLayout(false);
            this.groupBox1.PerformLayout();
            this.groupBox2.ResumeLayout(false);
            this.groupBox2.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Button okButton;
        private System.Windows.Forms.Button cancelButton;
        private System.Windows.Forms.GroupBox groupBox1;
        private System.Windows.Forms.CheckBox emitOptLlvmCheckBox;
        private System.Windows.Forms.CheckBox emitLlvmCheckBox;
        private System.Windows.Forms.GroupBox groupBox2;
        private System.Windows.Forms.CheckBox linkWithDebugRuntimeCheckBox;
    }
}