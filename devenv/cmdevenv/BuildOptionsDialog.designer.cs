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
            this.emitOptLlvmCheckBox = new System.Windows.Forms.CheckBox();
            this.emitLlvmCheckBox = new System.Windows.Forms.CheckBox();
            this.linkWithDebugRuntimeCheckBox = new System.Windows.Forms.CheckBox();
            this.groupBox3 = new System.Windows.Forms.GroupBox();
            this.strictNothrowCheckBox = new System.Windows.Forms.CheckBox();
            this.linkWithMsLinkCheckBox = new System.Windows.Forms.CheckBox();
            this.groupBox3.SuspendLayout();
            this.SuspendLayout();
            // 
            // okButton
            // 
            this.okButton.Location = new System.Drawing.Point(94, 184);
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
            this.cancelButton.Location = new System.Drawing.Point(175, 184);
            this.cancelButton.Name = "cancelButton";
            this.cancelButton.Size = new System.Drawing.Size(75, 23);
            this.cancelButton.TabIndex = 7;
            this.cancelButton.Text = "Cancel";
            this.cancelButton.UseVisualStyleBackColor = true;
            // 
            // emitOptLlvmCheckBox
            // 
            this.emitOptLlvmCheckBox.AutoSize = true;
            this.emitOptLlvmCheckBox.Location = new System.Drawing.Point(17, 74);
            this.emitOptLlvmCheckBox.Name = "emitOptLlvmCheckBox";
            this.emitOptLlvmCheckBox.Size = new System.Drawing.Size(211, 17);
            this.emitOptLlvmCheckBox.TabIndex = 1;
            this.emitOptLlvmCheckBox.Text = "Emit optimized LLVM intermediate code";
            this.emitOptLlvmCheckBox.UseVisualStyleBackColor = true;
            // 
            // emitLlvmCheckBox
            // 
            this.emitLlvmCheckBox.AutoSize = true;
            this.emitLlvmCheckBox.Location = new System.Drawing.Point(17, 51);
            this.emitLlvmCheckBox.Name = "emitLlvmCheckBox";
            this.emitLlvmCheckBox.Size = new System.Drawing.Size(164, 17);
            this.emitLlvmCheckBox.TabIndex = 0;
            this.emitLlvmCheckBox.Text = "Emit LLVM intermediate code";
            this.emitLlvmCheckBox.UseVisualStyleBackColor = true;
            // 
            // linkWithDebugRuntimeCheckBox
            // 
            this.linkWithDebugRuntimeCheckBox.AutoSize = true;
            this.linkWithDebugRuntimeCheckBox.Location = new System.Drawing.Point(17, 97);
            this.linkWithDebugRuntimeCheckBox.Name = "linkWithDebugRuntimeCheckBox";
            this.linkWithDebugRuntimeCheckBox.Size = new System.Drawing.Size(138, 17);
            this.linkWithDebugRuntimeCheckBox.TabIndex = 0;
            this.linkWithDebugRuntimeCheckBox.Text = "Link with debug runtime";
            this.linkWithDebugRuntimeCheckBox.UseVisualStyleBackColor = true;
            // 
            // groupBox3
            // 
            this.groupBox3.Controls.Add(this.linkWithMsLinkCheckBox);
            this.groupBox3.Controls.Add(this.linkWithDebugRuntimeCheckBox);
            this.groupBox3.Controls.Add(this.emitOptLlvmCheckBox);
            this.groupBox3.Controls.Add(this.strictNothrowCheckBox);
            this.groupBox3.Controls.Add(this.emitLlvmCheckBox);
            this.groupBox3.Location = new System.Drawing.Point(14, 13);
            this.groupBox3.Name = "groupBox3";
            this.groupBox3.Size = new System.Drawing.Size(236, 156);
            this.groupBox3.TabIndex = 14;
            this.groupBox3.TabStop = false;
            this.groupBox3.Text = "Compiling options";
            // 
            // strictNothrowCheckBox
            // 
            this.strictNothrowCheckBox.AutoSize = true;
            this.strictNothrowCheckBox.Location = new System.Drawing.Point(17, 28);
            this.strictNothrowCheckBox.Name = "strictNothrowCheckBox";
            this.strictNothrowCheckBox.Size = new System.Drawing.Size(91, 17);
            this.strictNothrowCheckBox.TabIndex = 0;
            this.strictNothrowCheckBox.Text = "Strict nothrow";
            this.strictNothrowCheckBox.UseVisualStyleBackColor = true;
            // 
            // linkWithMsLinkCheckBox
            // 
            this.linkWithMsLinkCheckBox.AutoSize = true;
            this.linkWithMsLinkCheckBox.Location = new System.Drawing.Point(17, 120);
            this.linkWithMsLinkCheckBox.Name = "linkWithMsLinkCheckBox";
            this.linkWithMsLinkCheckBox.Size = new System.Drawing.Size(160, 17);
            this.linkWithMsLinkCheckBox.TabIndex = 2;
            this.linkWithMsLinkCheckBox.Text = "Link with Microsoft\'s link.exe";
            this.linkWithMsLinkCheckBox.UseVisualStyleBackColor = true;
            // 
            // BuildOptionsDialog
            // 
            this.AcceptButton = this.okButton;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.CancelButton = this.cancelButton;
            this.ClientSize = new System.Drawing.Size(260, 221);
            this.Controls.Add(this.groupBox3);
            this.Controls.Add(this.cancelButton);
            this.Controls.Add(this.okButton);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog;
            this.MaximizeBox = false;
            this.MinimizeBox = false;
            this.Name = "BuildOptionsDialog";
            this.Text = "Build Options";
            this.groupBox3.ResumeLayout(false);
            this.groupBox3.PerformLayout();
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.Button okButton;
        private System.Windows.Forms.Button cancelButton;
        private System.Windows.Forms.CheckBox emitOptLlvmCheckBox;
        private System.Windows.Forms.CheckBox emitLlvmCheckBox;
        private System.Windows.Forms.CheckBox linkWithDebugRuntimeCheckBox;
        private System.Windows.Forms.GroupBox groupBox3;
        private System.Windows.Forms.CheckBox strictNothrowCheckBox;
        private System.Windows.Forms.CheckBox linkWithMsLinkCheckBox;
    }
}