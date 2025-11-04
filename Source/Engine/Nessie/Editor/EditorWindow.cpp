// EditorWindow.cpp
#include "EditorWindow.h"

namespace nes
{
    void EditorWindow::Deserialize(const YamlNode& in)
    {
        in["Name"].Read(m_desc.m_name);
    }

    void EditorWindow::Serialize(YamlWriter& out) const
    {
        out.Write("Name", m_desc.m_name);
    }
}
