
#ifndef _GTK_PRIVATE_TREEVIEW_H_
#define _GTK_PRIVATE_TREEVIEW_H_


class wxGtkTreePath
{
public:
        wxGtkTreePath(GtkTreePath *path = NULL) : m_path(path) { }

        wxGtkTreePath(const gchar *strpath)
        : m_path(gtk_tree_path_new_from_string(strpath))
    {
    }

        void Assign(GtkTreePath *path)
    {
        wxASSERT_MSG( !m_path, "shouldn't be already initialized" );

        m_path = path;
    }

            GtkTreePath **ByRef()
    {
        wxASSERT_MSG( !m_path, "shouldn't be already initialized" );

        return &m_path;
    }


    operator GtkTreePath *() const { return m_path; }

    ~wxGtkTreePath() { if ( m_path ) gtk_tree_path_free(m_path); }

private:
    GtkTreePath *m_path;

    wxDECLARE_NO_COPY_CLASS(wxGtkTreePath);
};

#endif 