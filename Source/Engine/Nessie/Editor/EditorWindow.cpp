// EditorWindow.cpp
#include "EditorWindow.h"

namespace nes
{
    void EditorWindow::SetWorld(const StrongPtr<EditorWorld>& world)
    {
        m_pWorld = world;
        OnWorldSet();
    }

    void EditorWindow::Deserialize(const YamlNode& in)
    {
        in["Name"].Read(m_desc.m_name);
        in["IsOpen"].Read(m_desc.m_isOpen, false);
    }

    void EditorWindow::Serialize(YamlOutStream& out) const
    {
        out.Write("Name", m_desc.m_name);
        out.Write("IsOpen", m_desc.m_isOpen);
    }
}
