namespace C3ComponentGenerator
{
    partial class ComponentGeneratorForm
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
            System.Windows.Forms.Label label1;
            System.Windows.Forms.Label label2;
            System.Windows.Forms.Label label3;
            this.GenerateButton = new System.Windows.Forms.Button();
            this.SourcePathEditor = new System.Windows.Forms.TextBox();
            this.IncludePathEditor = new System.Windows.Forms.TextBox();
            this.SourceFolderBrowserDlg = new System.Windows.Forms.FolderBrowserDialog();
            this.IncludeFolderBrowserDlg = new System.Windows.Forms.FolderBrowserDialog();
            this.BrowseSourceFolderButton = new System.Windows.Forms.Button();
            this.BrowseIncludeFolderButton = new System.Windows.Forms.Button();
            this.ComponentNameEditor = new System.Windows.Forms.TextBox();
            this.CelerityCoreCheck = new System.Windows.Forms.CheckBox();
            label1 = new System.Windows.Forms.Label();
            label2 = new System.Windows.Forms.Label();
            label3 = new System.Windows.Forms.Label();
            this.SuspendLayout();
            // 
            // label1
            // 
            label1.AutoSize = true;
            label1.Location = new System.Drawing.Point(12, 17);
            label1.Name = "label1";
            label1.Size = new System.Drawing.Size(120, 13);
            label1.TabIndex = 6;
            label1.Text = "Component Class Name";
            // 
            // label2
            // 
            label2.AutoSize = true;
            label2.Location = new System.Drawing.Point(12, 81);
            label2.Name = "label2";
            label2.Size = new System.Drawing.Size(121, 13);
            label2.TabIndex = 7;
            label2.Text = "Source Output Directory";
            // 
            // label3
            // 
            label3.AutoSize = true;
            label3.Location = new System.Drawing.Point(12, 145);
            label3.Name = "label3";
            label3.Size = new System.Drawing.Size(122, 13);
            label3.TabIndex = 8;
            label3.Text = "Include Output Directory";
            // 
            // GenerateButton
            // 
            this.GenerateButton.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
            this.GenerateButton.Location = new System.Drawing.Point(283, 276);
            this.GenerateButton.Name = "GenerateButton";
            this.GenerateButton.Size = new System.Drawing.Size(147, 23);
            this.GenerateButton.TabIndex = 0;
            this.GenerateButton.Text = "Generate";
            this.GenerateButton.UseVisualStyleBackColor = true;
            this.GenerateButton.Click += new System.EventHandler(this.GenerateButton_Click);
            // 
            // SourcePathEditor
            // 
            this.SourcePathEditor.AllowDrop = true;
            this.SourcePathEditor.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.SourcePathEditor.HideSelection = false;
            this.SourcePathEditor.Location = new System.Drawing.Point(15, 97);
            this.SourcePathEditor.Name = "SourcePathEditor";
            this.SourcePathEditor.Size = new System.Drawing.Size(383, 20);
            this.SourcePathEditor.TabIndex = 1;
            this.SourcePathEditor.WordWrap = false;
            // 
            // IncludePathEditor
            // 
            this.IncludePathEditor.AllowDrop = true;
            this.IncludePathEditor.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.IncludePathEditor.HideSelection = false;
            this.IncludePathEditor.Location = new System.Drawing.Point(15, 161);
            this.IncludePathEditor.Name = "IncludePathEditor";
            this.IncludePathEditor.Size = new System.Drawing.Size(383, 20);
            this.IncludePathEditor.TabIndex = 2;
            this.IncludePathEditor.WordWrap = false;
            // 
            // BrowseSourceFolderButton
            // 
            this.BrowseSourceFolderButton.Anchor = System.Windows.Forms.AnchorStyles.Right;
            this.BrowseSourceFolderButton.Location = new System.Drawing.Point(404, 96);
            this.BrowseSourceFolderButton.Name = "BrowseSourceFolderButton";
            this.BrowseSourceFolderButton.Size = new System.Drawing.Size(26, 23);
            this.BrowseSourceFolderButton.TabIndex = 3;
            this.BrowseSourceFolderButton.Text = "...";
            this.BrowseSourceFolderButton.UseVisualStyleBackColor = true;
            this.BrowseSourceFolderButton.Click += new System.EventHandler(this.BrowseSourceFolderButton_Click);
            // 
            // BrowseIncludeFolderButton
            // 
            this.BrowseIncludeFolderButton.Anchor = System.Windows.Forms.AnchorStyles.Right;
            this.BrowseIncludeFolderButton.Location = new System.Drawing.Point(404, 160);
            this.BrowseIncludeFolderButton.Name = "BrowseIncludeFolderButton";
            this.BrowseIncludeFolderButton.Size = new System.Drawing.Size(26, 23);
            this.BrowseIncludeFolderButton.TabIndex = 4;
            this.BrowseIncludeFolderButton.Text = "...";
            this.BrowseIncludeFolderButton.UseVisualStyleBackColor = true;
            this.BrowseIncludeFolderButton.Click += new System.EventHandler(this.BrowseIncludeFolderButton_Click);
            // 
            // ComponentNameEditor
            // 
            this.ComponentNameEditor.AllowDrop = true;
            this.ComponentNameEditor.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.ComponentNameEditor.HideSelection = false;
            this.ComponentNameEditor.Location = new System.Drawing.Point(15, 33);
            this.ComponentNameEditor.Name = "ComponentNameEditor";
            this.ComponentNameEditor.Size = new System.Drawing.Size(415, 20);
            this.ComponentNameEditor.TabIndex = 5;
            this.ComponentNameEditor.WordWrap = false;
            // 
            // CelerityCoreCheck
            // 
            this.CelerityCoreCheck.AutoSize = true;
            this.CelerityCoreCheck.Location = new System.Drawing.Point(15, 212);
            this.CelerityCoreCheck.Name = "CelerityCoreCheck";
            this.CelerityCoreCheck.Size = new System.Drawing.Size(85, 17);
            this.CelerityCoreCheck.TabIndex = 9;
            this.CelerityCoreCheck.Text = "Celerity Core";
            this.CelerityCoreCheck.UseVisualStyleBackColor = true;
            // 
            // ComponentGeneratorForm
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(442, 311);
            this.Controls.Add(this.CelerityCoreCheck);
            this.Controls.Add(label3);
            this.Controls.Add(label2);
            this.Controls.Add(label1);
            this.Controls.Add(this.ComponentNameEditor);
            this.Controls.Add(this.BrowseIncludeFolderButton);
            this.Controls.Add(this.BrowseSourceFolderButton);
            this.Controls.Add(this.IncludePathEditor);
            this.Controls.Add(this.SourcePathEditor);
            this.Controls.Add(this.GenerateButton);
            this.MaximumSize = new System.Drawing.Size(458, 350);
            this.Name = "ComponentGeneratorForm";
            this.Text = "Celerity Component Generator";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button GenerateButton;
        private System.Windows.Forms.TextBox SourcePathEditor;
        private System.Windows.Forms.TextBox IncludePathEditor;
        private System.Windows.Forms.FolderBrowserDialog SourceFolderBrowserDlg;
        private System.Windows.Forms.FolderBrowserDialog IncludeFolderBrowserDlg;
        private System.Windows.Forms.Button BrowseSourceFolderButton;
        private System.Windows.Forms.Button BrowseIncludeFolderButton;
        private System.Windows.Forms.TextBox ComponentNameEditor;
        private System.Windows.Forms.CheckBox CelerityCoreCheck;
    }
}

