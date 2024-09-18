using C3ComponentGenerator;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Xml.Linq;

namespace C3ComponentGenerator
{
    public partial class ComponentGeneratorForm : Form
    {
        public ComponentGeneratorForm()
        {
            InitializeComponent();
        }

        private void BrowseSourceFolderButton_Click(object sender, EventArgs e)
        {
            if (SourcePathEditor.Text.Length > 0)
            {
                SourceFolderBrowserDlg.SelectedPath = SourcePathEditor.Text;
            }

            if (SourceFolderBrowserDlg.ShowDialog(this) == DialogResult.OK)
            {
                SourcePathEditor.Text = SourceFolderBrowserDlg.SelectedPath;
            }
        }

        private void BrowseIncludeFolderButton_Click(object sender, EventArgs e)
        {
            if (IncludePathEditor.Text.Length > 0)
            {
                IncludeFolderBrowserDlg.SelectedPath = IncludePathEditor.Text;
            }

            if (IncludeFolderBrowserDlg.ShowDialog(this) == DialogResult.OK)
            {
                IncludePathEditor.Text = IncludeFolderBrowserDlg.SelectedPath;
            }
        }

        private void GenerateButton_Click(object sender, EventArgs e)
        {
            String name = ComponentNameEditor.Text;
            String impl_name = name + "Impl";

            String inc_name = ((CelerityCoreCheck.Checked == true) ? "C3" : "") + name + ".h";
            String inc_path = IncludePathEditor.Text + "/" + inc_name;

            String inc_impl_name = ((CelerityCoreCheck.Checked == true) ? "C3" : "") + name + "Impl.h";
            String inc_impl_path = SourcePathEditor.Text + "/" + inc_impl_name;

            String src_impl_path = SourcePathEditor.Text + "/" + ((CelerityCoreCheck.Checked == true) ? "C3" : "") + name + "Impl.cpp";

            String inc =
                "// TODO: add copyright\n\n" +
                "#pragma once\n\n" +
                "#include <C3.h>\n\n" +
                "namespace c3\n{\n" +
                "\tclass " + name + " : public Component\n\t{\n\n\tpublic:\n\n" +
                "\t\tstatic const ComponentType* Type();\n\n" +
                "\t};\n\n};\n";

            if (!File.Exists(inc_path) || (MessageBox.Show(
                "\"" + inc_path + "\" already exists; do you want to overwrite it?",
                "Overwrite File?", MessageBoxButtons.YesNo) == DialogResult.Yes))
            {
                File.WriteAllText(inc_path, inc);
            }

            System.Guid g = System.Guid.NewGuid();

            String inc_impl =
                "// TODO: add copyright\n\n" +
                "#pragma once\n\n" +
                "#include <C3.h>\n" +
                "#include <" + inc_name + ">\n\n" +
                "namespace c3\n{\n\n" +
                "\tclass " + impl_name + " : public " + name + ", props::IPropertyChangeListener\n\t{\n" +
                "\tprotected:\n\n" +
                "\t\tObject* m_pOwner;\n\n" +
                "\t\tprops::TFlags64 m_Flags;\n\n" +
                "\tpublic:\n\n" +
                "\t\t" + impl_name + "();\n\n" +
                "\t\tvirtual ~" + impl_name + "();\n\n" +
                "\t\tvirtual void Release();\n\n" +
                "\t\tvirtual const ComponentType *GetType() const;\n\n" +
                "\t\tvirtual props::TFlags64 Flags() const;\n\n" +
                "\t\tvirtual bool Initialize(Object *pobject);\n\n" +
                "\t\tvirtual void Update(float elapsed_time = 0.0f);\n\n" +
				"\t\tvirtual bool Prerender(Object::RenderFlags flags, int draworder = 0);\n\n" +
				"\t\tvirtual void Render(Object::RenderFlags rendflags, const glm::fmat4x4 *pmat);\n\n" +
                "\t\tvirtual void PropertyChanged(const props::IProperty *pprop);\n\n" +
				"\t\tvirtual bool Intersect(const glm::vec3 *pRayPos, const glm::vec3 *pRayDir, const glm::fmat4x4 *mats, float *pDistance) const;\n\n" +
                "\t};\n\n" +
                "\tDEFINE_COMPONENTTYPE(" + name + ", " + impl_name + ", GUID(" + g.ToString("X") + "), \"" + name + "\", \"TODO: describe your component here\", 0);\n\n" +
                "};\n";

            if (!File.Exists(inc_impl_path) || (MessageBox.Show(
                "\"" + inc_impl_path + "\" already exists; do you want to overwrite it?",
                "Overwrite File?", MessageBoxButtons.YesNo) == DialogResult.Yes))
            {
                File.WriteAllText(inc_impl_path, inc_impl);
            }

            String src_impl =
                "// TODO: add copyright\n\n" +
                "#include \"pch.h\"\n\n" +
                "#include \"" + inc_impl_name + "\"\n\n" +
                "using namespace c3;\n\n\n" +
                "DECLARE_COMPONENTTYPE(" + name + ", " + impl_name + ");\n\n\n" +
                impl_name + "::" + impl_name + "()\n{\n}\n\n\n" +
                impl_name + "::~" + impl_name + "()\n{\n}\n\n\n" +
                "void " + impl_name + "::Release()\n{\n\tdelete this;\n}\n\n\n" +
                "props::TFlags64 " + impl_name + "::Flags() const\n{\n\treturn 0;\n}\n\n\n" +

                "bool " + impl_name + "::Initialize(Object *pobject)\n{\n" +
                "\tif (nullptr == (m_pOwner = pobject))\n\t\treturn false;\n\n" +
                "\tprops::IPropertySet* propset = pobject->GetProperties();\n" +
                "\tif (!propset)\n\t\treturn false;\n\n\t// TODO: create your required properties here...\n\n" +
                "\treturn true;\n}\n\n\n" +

                "void " + impl_name + "::Update(float elapsed_time)\n{\n\tif (elapsed_time == 0)\n\t\treturn;\n}\n\n\n" +

                "bool " + impl_name + "::Prerender(Object::RenderFlags flags, int draworder)\n{\n" +
                "\tif (flags.IsSet(RF_FORCE))\n\t\treturn true;\n\n" +
                "\tif (!m_pOwner->Flags().IsSet(OF_DRAW))\n\t\treturn false;\n\n\treturn true;\n}\n\n\n" +

                "void " + impl_name + "::Render(Object::RenderFlags flags, const glm::fmat4x4 *pmat)\n{\n" +
                "\tif (flags.IsSet(RF_SHADOW))\n\t\treturn; // TODO; just suggestions\n\n" +
                "\tif (flags.IsSet(RF_LIGHT))\n\t\treturn; // TODO; just suggestions\n\n" +
                "\tif (flags.IsSet(RF_AUXILIARY))\n\t\treturn; // TODO; just suggestions\n\n" +
                "\tRenderer *pr = m_pOwner->GetSystem()->GetRenderer();\n\n" +
                "\t//TODO: use pr to draw thing(s)\n\n}\n\n\n" +

                "void " + impl_name + "::PropertyChanged(const props::IProperty *pprop)\n{\n" +
                "\tprops::FOURCHARCODE fcc = pprop->GetID();\n\n" +
				"#if 0  // include this code once you need to check for property changes\n" +
                "\tswitch (fcc) // TODO: handle property changes by ID\n\t{\n\t\tdefault:\n\t\t\tbreak;\n\t}\n" +
				"#endif\n" +
				"}\n\n\n" +

                "bool " + impl_name + "::Intersect(const glm::vec3 *pRayPos, const glm::vec3 *pRayDir, const glm::fmat4x4 *mats, float *pDistance) const\n" +
                "{\n\tbool ret = false;\n\n\t// TODO: check collisions however you want here\n\n\treturn ret;\n}\n";

            if (!File.Exists(src_impl_path) || (MessageBox.Show(
                "\"" + src_impl_path + "\" already exists; do you want to overwrite it?",
                "Overwrite File?", MessageBoxButtons.YesNo) == DialogResult.Yes))
            {
                File.WriteAllText(src_impl_path, src_impl);
            }

            MessageBox.Show("Your Component-derived class has been generated. Remember to un/register it in your plug-in or c3::SystemImpl.");
        }
    }
}
