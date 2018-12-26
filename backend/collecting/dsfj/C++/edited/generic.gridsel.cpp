
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif



#if wxUSE_GRID

#include "wx/generic/gridsel.h"



wxGridSelection::wxGridSelection( wxGrid * grid,
                                  wxGrid::wxGridSelectionModes sel )
{
    m_grid = grid;
    m_selectionMode = sel;
}

bool wxGridSelection::IsSelection()
{
  return ( m_cellSelection.GetCount() || m_blockSelectionTopLeft.GetCount() ||
           m_rowSelection.GetCount() || m_colSelection.GetCount() );
}

bool wxGridSelection::IsInSelection( int row, int col )
{
    size_t count;

            if ( m_selectionMode == wxGrid::wxGridSelectCells )
    {
        count = m_cellSelection.GetCount();
        for ( size_t n = 0; n < count; n++ )
        {
            wxGridCellCoords& coords = m_cellSelection[n];
            if ( row == coords.GetRow() && col == coords.GetCol() )
                return true;
        }
    }

            count = m_blockSelectionTopLeft.GetCount();
    for ( size_t n = 0; n < count; n++ )
    {
        wxGridCellCoords& coords1 = m_blockSelectionTopLeft[n];
        wxGridCellCoords& coords2 = m_blockSelectionBottomRight[n];
        if ( BlockContainsCell(coords1.GetRow(), coords1.GetCol(),
                               coords2.GetRow(), coords2.GetCol(),
                               row, col ) )
            return true;
    }

                if ( m_selectionMode != wxGrid::wxGridSelectColumns )
    {
        count = m_rowSelection.GetCount();
        for ( size_t n = 0; n < count; n++ )
        {
            if ( row == m_rowSelection[n] )
              return true;
        }
    }

                if ( m_selectionMode != wxGrid::wxGridSelectRows )
    {
        count = m_colSelection.GetCount();
        for ( size_t n = 0; n < count; n++ )
        {
            if ( col == m_colSelection[n] )
              return true;
        }
    }

    return false;
}

void wxGridSelection::SetSelectionMode( wxGrid::wxGridSelectionModes selmode )
{
        if (selmode == m_selectionMode)
        return;

    if ( m_selectionMode != wxGrid::wxGridSelectCells )
    {
                        if ( selmode != wxGrid::wxGridSelectCells )
            ClearSelection();

        m_selectionMode = selmode;
    }
    else
    {
                        size_t n;
        while ( ( n = m_cellSelection.GetCount() ) > 0 )
        {
            n--;
            wxGridCellCoords& coords = m_cellSelection[n];
            int row = coords.GetRow();
            int col = coords.GetCol();
            m_cellSelection.RemoveAt(n);
            if (selmode == wxGrid::wxGridSelectRows)
                SelectRow( row );
            else                 SelectCol( col );
        }

                for (n = 0; n < m_blockSelectionTopLeft.GetCount(); n++)
        {
            wxGridCellCoords& coords = m_blockSelectionTopLeft[n];
            int topRow = coords.GetRow();
            int leftCol = coords.GetCol();
            coords = m_blockSelectionBottomRight[n];
            int bottomRow = coords.GetRow();
            int rightCol = coords.GetCol();

            if (selmode == wxGrid::wxGridSelectRows)
            {
                if (leftCol != 0 || rightCol != m_grid->GetNumberCols() - 1 )
                {
                    m_blockSelectionTopLeft.RemoveAt(n);
                    m_blockSelectionBottomRight.RemoveAt(n);
                    SelectBlockNoEvent( topRow, 0,
                                 bottomRow, m_grid->GetNumberCols() - 1);
                }
            }
            else             {
                if (topRow != 0 || bottomRow != m_grid->GetNumberRows() - 1 )
                {
                    m_blockSelectionTopLeft.RemoveAt(n);
                    m_blockSelectionBottomRight.RemoveAt(n);
                    SelectBlockNoEvent(0, leftCol,
                                 m_grid->GetNumberRows() - 1, rightCol);
                }
            }
        }

        m_selectionMode = selmode;
    }
}

void wxGridSelection::SelectRow(int row, const wxKeyboardState& kbd)
{
    if ( m_selectionMode == wxGrid::wxGridSelectColumns )
        return;

    size_t count, n;

        if ( m_selectionMode == wxGrid::wxGridSelectCells )
    {
        count = m_cellSelection.GetCount();
        for ( n = 0; n < count; n++ )
        {
            wxGridCellCoords& coords = m_cellSelection[n];
            if ( BlockContainsCell( row, 0, row, m_grid->GetNumberCols() - 1,
                                    coords.GetRow(), coords.GetCol() ) )
            {
                m_cellSelection.RemoveAt(n);
                n--;
                count--;
            }
        }
    }

        count = m_blockSelectionTopLeft.GetCount();
    bool done = false;

    for ( n = 0; n < count; n++ )
    {
        wxGridCellCoords& coords1 = m_blockSelectionTopLeft[n];
        wxGridCellCoords& coords2 = m_blockSelectionBottomRight[n];

                if ( coords1.GetRow() == row && row == coords2.GetRow() )
        {
            m_blockSelectionTopLeft.RemoveAt(n);
            m_blockSelectionBottomRight.RemoveAt(n);
            n--;
            count--;
        }
        else if ( coords1.GetCol() == 0  &&
                  coords2.GetCol() == m_grid->GetNumberCols() - 1 )
        {
                        if ( coords1.GetRow() <= row && row <= coords2.GetRow() )
                return;
                        else if ( coords1.GetRow() == row + 1)
            {
                coords1.SetRow(row);
                done = true;
            }
            else if ( coords2.GetRow() == row - 1)
            {
                coords2.SetRow(row);
                done = true;
            }
        }
    }

            if ( !done )
    {
        count = m_rowSelection.GetCount();
        for ( n = 0; n < count; n++ )
        {
            if ( row == m_rowSelection[n] )
                return;
        }

                m_rowSelection.Add(row);
    }

        if ( !m_grid->GetBatchCount() )
    {
        wxRect r = m_grid->BlockToDeviceRect( wxGridCellCoords( row, 0 ),
                                              wxGridCellCoords( row, m_grid->GetNumberCols() - 1 ) );
        ((wxWindow *)m_grid->m_gridWin)->Refresh( false, &r );
    }

        wxGridRangeSelectEvent gridEvt( m_grid->GetId(),
                                    wxEVT_GRID_RANGE_SELECT,
                                    m_grid,
                                    wxGridCellCoords( row, 0 ),
                                    wxGridCellCoords( row, m_grid->GetNumberCols() - 1 ),
                                    true,
                                    kbd);

    m_grid->GetEventHandler()->ProcessEvent( gridEvt );
}

void wxGridSelection::SelectCol(int col, const wxKeyboardState& kbd)
{
    if ( m_selectionMode == wxGrid::wxGridSelectRows )
        return;
    size_t count, n;

        if ( m_selectionMode == wxGrid::wxGridSelectCells )
    {
        count = m_cellSelection.GetCount();
        for ( n = 0; n < count; n++ )
        {
            wxGridCellCoords& coords = m_cellSelection[n];
            if ( BlockContainsCell( 0, col, m_grid->GetNumberRows() - 1, col,
                                    coords.GetRow(), coords.GetCol() ) )
            {
                m_cellSelection.RemoveAt(n);
                n--;
                count--;
            }
        }
    }

        count = m_blockSelectionTopLeft.GetCount();
    bool done = false;
    for ( n = 0; n < count; n++ )
    {
        wxGridCellCoords& coords1 = m_blockSelectionTopLeft[n];
        wxGridCellCoords& coords2 = m_blockSelectionBottomRight[n];

                if ( coords1.GetCol() == col && col == coords2.GetCol() )
        {
            m_blockSelectionTopLeft.RemoveAt(n);
            m_blockSelectionBottomRight.RemoveAt(n);
            n--;
            count--;
        }
        else if ( coords1.GetRow() == 0  &&
                  coords2.GetRow() == m_grid->GetNumberRows() - 1 )
        {
                        if ( coords1.GetCol() <= col && col <= coords2.GetCol() )
                return;
                        else if ( coords1.GetCol() == col + 1)
            {
                coords1.SetCol(col);
                done = true;
            }
            else if ( coords2.GetCol() == col - 1)
            {
                coords2.SetCol(col);
                done = true;
            }
        }
    }

            if ( !done )
    {
        count = m_colSelection.GetCount();
        for ( n = 0; n < count; n++ )
        {
            if ( col == m_colSelection[n] )
                return;
        }

                m_colSelection.Add(col);
    }

        if ( !m_grid->GetBatchCount() )
    {
        wxRect r = m_grid->BlockToDeviceRect( wxGridCellCoords( 0, col ),
                                              wxGridCellCoords( m_grid->GetNumberRows() - 1, col ) );
        ((wxWindow *)m_grid->m_gridWin)->Refresh( false, &r );
    }

        wxGridRangeSelectEvent gridEvt( m_grid->GetId(),
                                    wxEVT_GRID_RANGE_SELECT,
                                    m_grid,
                                    wxGridCellCoords( 0, col ),
                                    wxGridCellCoords( m_grid->GetNumberRows() - 1, col ),
                                    true,
                                    kbd );

    m_grid->GetEventHandler()->ProcessEvent( gridEvt );
}

void wxGridSelection::SelectBlock( int topRow, int leftCol,
                                   int bottomRow, int rightCol,
                                   const wxKeyboardState& kbd,
                                   bool sendEvent )
{
        switch ( m_selectionMode )
    {
        default:
            wxFAIL_MSG( "unknown selection mode" );
            wxFALLTHROUGH;

        case wxGrid::wxGridSelectCells:
                        break;

        case wxGrid::wxGridSelectRows:
            leftCol = 0;
            rightCol = m_grid->GetNumberCols() - 1;
            break;

        case wxGrid::wxGridSelectColumns:
            topRow = 0;
            bottomRow = m_grid->GetNumberRows() - 1;
            break;

        case wxGrid::wxGridSelectRowsOrColumns:
                                    return;
    }

    if ( topRow > bottomRow )
    {
        int temp = topRow;
        topRow = bottomRow;
        bottomRow = temp;
    }

    if ( leftCol > rightCol )
    {
        int temp = leftCol;
        leftCol = rightCol;
        rightCol = temp;
    }

                    if ( m_selectionMode == wxGrid::wxGridSelectCells &&
         topRow == bottomRow && leftCol == rightCol )
    {
        SelectCell( topRow, leftCol, kbd, sendEvent );
    }

    size_t count, n;

    if ( m_selectionMode == wxGrid::wxGridSelectRows )
    {
                wxArrayInt alreadyselected;
        alreadyselected.Add(0,bottomRow-topRow+1);
        for( n = 0; n < m_rowSelection.GetCount(); n++)
        {
            int row = m_rowSelection[n];
            if( (row >= topRow) && (row <= bottomRow) )
            {
                alreadyselected[ row - topRow ]=1;
            }
        }

                for ( int row = topRow; row <= bottomRow; row++ )
        {
            if ( alreadyselected[ row - topRow ] == 0 )
            {
                m_rowSelection.Add( row );
            }
        }
    }
    else if ( m_selectionMode == wxGrid::wxGridSelectColumns )
    {
                wxArrayInt alreadyselected;
        alreadyselected.Add(0,rightCol-leftCol+1);
        for( n = 0; n < m_colSelection.GetCount(); n++)
        {
            int col = m_colSelection[n];
            if( (col >= leftCol) && (col <= rightCol) )
            {
                alreadyselected[ col - leftCol ]=1;
            }
        }

                for ( int col = leftCol; col <= rightCol; col++ )
        {
            if ( alreadyselected[ col - leftCol ] == 0 )
            {
                m_colSelection.Add( col );
            }
        }
    }
    else
    {
                if ( m_selectionMode == wxGrid::wxGridSelectCells )
        {
            count = m_cellSelection.GetCount();
            for ( n = 0; n < count; n++ )
            {
                wxGridCellCoords& coords = m_cellSelection[n];
                if ( BlockContainsCell( topRow, leftCol, bottomRow, rightCol,
                                        coords.GetRow(), coords.GetCol() ) )
                {
                    m_cellSelection.RemoveAt(n);
                    n--;
                    count--;
                }
            }
        }

                
        count = m_blockSelectionTopLeft.GetCount();
        for ( n = 0; n < count; n++ )
        {
            wxGridCellCoords& coords1 = m_blockSelectionTopLeft[n];
            wxGridCellCoords& coords2 = m_blockSelectionBottomRight[n];

            switch ( BlockContain( coords1.GetRow(), coords1.GetCol(),
                                   coords2.GetRow(), coords2.GetCol(),
                                   topRow, leftCol, bottomRow, rightCol ) )
            {
                case 1:
                    return;

                case -1:
                    m_blockSelectionTopLeft.RemoveAt(n);
                    m_blockSelectionBottomRight.RemoveAt(n);
                    n--;
                    count--;
                    break;

                default:
                    break;
            }
        }

                        count = m_rowSelection.GetCount();
        for ( n = 0; n < count; n++ )
        {
            switch ( BlockContain( m_rowSelection[n], 0,
                                   m_rowSelection[n], m_grid->GetNumberCols() - 1,
                                   topRow, leftCol, bottomRow, rightCol ) )
            {
                case 1:
                    return;

                case -1:
                    m_rowSelection.RemoveAt(n);
                    n--;
                    count--;
                    break;

                default:
                    break;
            }
        }

                count = m_colSelection.GetCount();
        for ( n = 0; n < count; n++ )
        {
            switch ( BlockContain( 0, m_colSelection[n],
                                   m_grid->GetNumberRows() - 1, m_colSelection[n],
                                   topRow, leftCol, bottomRow, rightCol ) )
            {
                case 1:
                    return;

                case -1:
                    m_colSelection.RemoveAt(n);
                    n--;
                    count--;
                    break;

                default:
                    break;
            }
        }

        m_blockSelectionTopLeft.Add( wxGridCellCoords( topRow, leftCol ) );
        m_blockSelectionBottomRight.Add( wxGridCellCoords( bottomRow, rightCol ) );
    }
        if ( !m_grid->GetBatchCount() )
    {
        wxRect r = m_grid->BlockToDeviceRect( wxGridCellCoords( topRow, leftCol ),
                                              wxGridCellCoords( bottomRow, rightCol ) );
        ((wxWindow *)m_grid->m_gridWin)->Refresh( false, &r );
    }

        if ( sendEvent )
    {
        wxGridRangeSelectEvent gridEvt( m_grid->GetId(),
            wxEVT_GRID_RANGE_SELECT,
            m_grid,
            wxGridCellCoords( topRow, leftCol ),
            wxGridCellCoords( bottomRow, rightCol ),
            true,
            kbd);
        m_grid->GetEventHandler()->ProcessEvent( gridEvt );
    }
}

void wxGridSelection::SelectCell( int row, int col,
                                  const wxKeyboardState& kbd,
                                  bool sendEvent )
{
    if ( IsInSelection ( row, col ) )
        return;

    wxGridCellCoords selectedTopLeft, selectedBottomRight;
    if ( m_selectionMode == wxGrid::wxGridSelectRows )
    {
        m_rowSelection.Add( row );
        selectedTopLeft = wxGridCellCoords( row, 0 );
        selectedBottomRight = wxGridCellCoords( row, m_grid->GetNumberCols() - 1 );
    }
    else if ( m_selectionMode == wxGrid::wxGridSelectColumns )
    {
        m_colSelection.Add( col );
        selectedTopLeft = wxGridCellCoords( 0, col );
        selectedBottomRight = wxGridCellCoords( m_grid->GetNumberRows() - 1, col );
    }
    else
    {
        m_cellSelection.Add( wxGridCellCoords( row, col ) );
        selectedTopLeft = wxGridCellCoords( row, col );
        selectedBottomRight = wxGridCellCoords( row, col );
    }

        if ( !m_grid->GetBatchCount() )
    {
        wxRect r = m_grid->BlockToDeviceRect(
            selectedTopLeft,
            selectedBottomRight );
        ((wxWindow *)m_grid->m_gridWin)->Refresh( false, &r );
    }

        if (sendEvent)
    {
        wxGridRangeSelectEvent gridEvt( m_grid->GetId(),
            wxEVT_GRID_RANGE_SELECT,
            m_grid,
            selectedTopLeft,
            selectedBottomRight,
            true,
            kbd);
        m_grid->GetEventHandler()->ProcessEvent( gridEvt );
    }
}

void
wxGridSelection::ToggleCellSelection(int row, int col,
                                     const wxKeyboardState& kbd)
{
        if ( !IsInSelection ( row, col ) )
    {
        SelectCell(row, col, kbd);

        return;
    }

            size_t count, n;

                
    if ( m_selectionMode == wxGrid::wxGridSelectCells )
    {
        count = m_cellSelection.GetCount();
        for ( n = 0; n < count; n++ )
        {
            const wxGridCellCoords& sel = m_cellSelection[n];
            if ( row == sel.GetRow() && col == sel.GetCol() )
            {
                wxGridCellCoords coords = m_cellSelection[n];
                m_cellSelection.RemoveAt(n);
                if ( !m_grid->GetBatchCount() )
                {
                    wxRect r = m_grid->BlockToDeviceRect( coords, coords );
                    ((wxWindow *)m_grid->m_gridWin)->Refresh( false, &r );
                }

                                wxGridRangeSelectEvent gridEvt( m_grid->GetId(),
                                                wxEVT_GRID_RANGE_SELECT,
                                                m_grid,
                                                wxGridCellCoords( row, col ),
                                                wxGridCellCoords( row, col ),
                                                false,
                                                kbd );
                m_grid->GetEventHandler()->ProcessEvent( gridEvt );

                return;
            }
        }
    }

                                                                        
    count = m_blockSelectionTopLeft.GetCount();
    for ( n = 0; n < count; n++ )
    {
        wxGridCellCoords& coords1 = m_blockSelectionTopLeft[n];
        wxGridCellCoords& coords2 = m_blockSelectionBottomRight[n];
        int topRow = coords1.GetRow();
        int leftCol = coords1.GetCol();
        int bottomRow = coords2.GetRow();
        int rightCol = coords2.GetCol();

        if ( BlockContainsCell( topRow, leftCol, bottomRow, rightCol, row, col ) )
        {
                        m_blockSelectionTopLeft.RemoveAt(n);
            m_blockSelectionBottomRight.RemoveAt(n);
            n--;
            count--;

                        if ( m_selectionMode != wxGrid::wxGridSelectColumns )
            {
                if ( topRow < row )
                    SelectBlockNoEvent(topRow, leftCol, row - 1, rightCol);
                if ( bottomRow > row )
                    SelectBlockNoEvent(row + 1, leftCol, bottomRow, rightCol);
            }

            if ( m_selectionMode != wxGrid::wxGridSelectRows )
            {
                if ( leftCol < col )
                    SelectBlockNoEvent(row, leftCol, row, col - 1);
                if ( rightCol > col )
                    SelectBlockNoEvent(row, col + 1, row, rightCol);
            }
        }
    }

    bool rowSelectionWasChanged = false;
        if ( m_selectionMode != wxGrid::wxGridSelectColumns )
    {
        count = m_rowSelection.GetCount();
        for ( n = 0; n < count; n++ )
        {
            if ( m_rowSelection[n] == row )
            {
                m_rowSelection.RemoveAt(n);
                n--;
                count--;

                rowSelectionWasChanged = true;

                if (m_selectionMode == wxGrid::wxGridSelectCells)
                {
                    if ( col > 0 )
                        SelectBlockNoEvent(row, 0, row, col - 1);
                    if ( col < m_grid->GetNumberCols() - 1 )
                        SelectBlockNoEvent( row, col + 1,
                                     row, m_grid->GetNumberCols() - 1);
                }
            }
        }
    }

    bool colSelectionWasChanged = false;
        if ( m_selectionMode != wxGrid::wxGridSelectRows )
    {
        count = m_colSelection.GetCount();
        for ( n = 0; n < count; n++ )
        {
            if ( m_colSelection[n] == col )
            {
                m_colSelection.RemoveAt(n);
                n--;
                count--;

                colSelectionWasChanged = true;

                if (m_selectionMode == wxGrid::wxGridSelectCells)
                {
                    if ( row > 0 )
                        SelectBlockNoEvent(0, col, row - 1, col);
                    if ( row < m_grid->GetNumberRows() - 1 )
                        SelectBlockNoEvent(row + 1, col,
                                     m_grid->GetNumberRows() - 1, col);
                }
            }
        }
    }

            wxRect r;
    if ( m_selectionMode == wxGrid::wxGridSelectCells )
    {
        if ( !m_grid->GetBatchCount() )
        {
            r = m_grid->BlockToDeviceRect(
                wxGridCellCoords( row, col ),
                wxGridCellCoords( row, col ) );
            ((wxWindow *)m_grid->m_gridWin)->Refresh( false, &r );
        }

        wxGridRangeSelectEvent gridEvt( m_grid->GetId(),
            wxEVT_GRID_RANGE_SELECT,
            m_grid,
            wxGridCellCoords( row, col ),
            wxGridCellCoords( row, col ),
            false,
            kbd );
        m_grid->GetEventHandler()->ProcessEvent( gridEvt );
    }
    else      {
        if ( m_selectionMode != wxGrid::wxGridSelectColumns &&
                rowSelectionWasChanged )
        {
            int numCols = m_grid->GetNumberCols();
            for ( int colFrom = 0, colTo = 0; colTo <= numCols; ++colTo )
            {
                if ( m_colSelection.Index(colTo) >= 0 || colTo == numCols )
                {
                    if ( colFrom < colTo )
                    {
                        if ( !m_grid->GetBatchCount() )
                        {
                            r = m_grid->BlockToDeviceRect(
                                wxGridCellCoords( row, colFrom ),
                                wxGridCellCoords( row, colTo-1 ) );
                            ((wxWindow *)m_grid->m_gridWin)->Refresh( false, &r );
                        }

                        wxGridRangeSelectEvent gridEvt( m_grid->GetId(),
                            wxEVT_GRID_RANGE_SELECT,
                            m_grid,
                            wxGridCellCoords( row, colFrom ),
                            wxGridCellCoords( row, colTo - 1 ),
                            false,
                            kbd );
                        m_grid->GetEventHandler()->ProcessEvent( gridEvt );
                    }

                    colFrom = colTo + 1;
                }
            }
        }

        if ( m_selectionMode != wxGrid::wxGridSelectRows &&
                colSelectionWasChanged )
        {
            int numRows = m_grid->GetNumberRows();
            for ( int rowFrom = 0, rowTo = 0; rowTo <= numRows; ++rowTo )
            {
                if ( m_rowSelection.Index(rowTo) >= 0 || rowTo == numRows )
                {
                    if (rowFrom < rowTo)
                    {
                        if ( !m_grid->GetBatchCount() )
                        {
                            r = m_grid->BlockToDeviceRect(
                                wxGridCellCoords( rowFrom, col ),
                                wxGridCellCoords( rowTo - 1, col ) );
                          ((wxWindow *)m_grid->m_gridWin)->Refresh( false, &r );
                        }

                        wxGridRangeSelectEvent gridEvt( m_grid->GetId(),
                            wxEVT_GRID_RANGE_SELECT,
                            m_grid,
                            wxGridCellCoords( rowFrom, col ),
                            wxGridCellCoords( rowTo - 1, col ),
                            false,
                            kbd );
                        m_grid->GetEventHandler()->ProcessEvent( gridEvt );
                    }

                    rowFrom = rowTo + 1;
                }
            }
        }
    }
}

void wxGridSelection::ClearSelection()
{
    size_t n;
    wxRect r;
    wxGridCellCoords coords1, coords2;

        if ( m_selectionMode == wxGrid::wxGridSelectCells )
    {
        while ( ( n = m_cellSelection.GetCount() ) > 0)
        {
            n--;
            coords1 = m_cellSelection[n];
            m_cellSelection.RemoveAt(n);
            if ( !m_grid->GetBatchCount() )
            {
                r = m_grid->BlockToDeviceRect( coords1, coords1 );
                ((wxWindow *)m_grid->m_gridWin)->Refresh( false, &r );

#ifdef __WXMAC__
                ((wxWindow *)m_grid->m_gridWin)->Update();
#endif
            }
        }
    }

        while ( ( n = m_blockSelectionTopLeft.GetCount() ) > 0)
    {
        n--;
        coords1 = m_blockSelectionTopLeft[n];
        coords2 = m_blockSelectionBottomRight[n];
        m_blockSelectionTopLeft.RemoveAt(n);
        m_blockSelectionBottomRight.RemoveAt(n);
        if ( !m_grid->GetBatchCount() )
        {
            r = m_grid->BlockToDeviceRect( coords1, coords2 );
            ((wxWindow *)m_grid->m_gridWin)->Refresh( false, &r );

#ifdef __WXMAC__
            ((wxWindow *)m_grid->m_gridWin)->Update();
#endif
        }
    }

        if ( m_selectionMode != wxGrid::wxGridSelectColumns )
    {
        while ( ( n = m_rowSelection.GetCount() ) > 0)
        {
            n--;
            int row = m_rowSelection[n];
            m_rowSelection.RemoveAt(n);
            if ( !m_grid->GetBatchCount() )
            {
                r = m_grid->BlockToDeviceRect( wxGridCellCoords( row, 0 ),
                                               wxGridCellCoords( row, m_grid->GetNumberCols() - 1 ) );
                ((wxWindow *)m_grid->m_gridWin)->Refresh( false, &r );

#ifdef __WXMAC__
                ((wxWindow *)m_grid->m_gridWin)->Update();
#endif
            }
        }
    }

        if ( m_selectionMode != wxGrid::wxGridSelectRows )
    {
        while ( ( n = m_colSelection.GetCount() ) > 0)
        {
            n--;
            int col = m_colSelection[n];
            m_colSelection.RemoveAt(n);
            if ( !m_grid->GetBatchCount() )
            {
                r = m_grid->BlockToDeviceRect( wxGridCellCoords( 0, col ),
                                               wxGridCellCoords( m_grid->GetNumberRows() - 1, col ) );
                ((wxWindow *)m_grid->m_gridWin)->Refresh( false, &r );

#ifdef __WXMAC__
                ((wxWindow *)m_grid->m_gridWin)->Update();
#endif
            }
        }
    }

                wxGridRangeSelectEvent gridEvt( m_grid->GetId(),
                                    wxEVT_GRID_RANGE_SELECT,
                                    m_grid,
                                    wxGridCellCoords( 0, 0 ),
                                    wxGridCellCoords(
                                        m_grid->GetNumberRows() - 1,
                                        m_grid->GetNumberCols() - 1 ),
                                    false );

    m_grid->GetEventHandler()->ProcessEvent(gridEvt);
}


void wxGridSelection::UpdateRows( size_t pos, int numRows )
{
    size_t count = m_cellSelection.GetCount();
    size_t n;
    for ( n = 0; n < count; n++ )
    {
        wxGridCellCoords& coords = m_cellSelection[n];
        wxCoord row = coords.GetRow();
        if ((size_t)row >= pos)
        {
            if (numRows > 0)
            {
                                coords.SetRow(row + numRows);
            }
            else if (numRows < 0)
            {
                                if ((size_t)row >= pos - numRows)
                {
                                        coords.SetRow(row + numRows);
                }
                else
                {
                                        m_cellSelection.RemoveAt(n);
                    n--;
                    count--;
                }
            }
        }
    }

    count = m_blockSelectionTopLeft.GetCount();
    for ( n = 0; n < count; n++ )
    {
        wxGridCellCoords& coords1 = m_blockSelectionTopLeft[n];
        wxGridCellCoords& coords2 = m_blockSelectionBottomRight[n];
        wxCoord row1 = coords1.GetRow();
        wxCoord row2 = coords2.GetRow();

        if ((size_t)row2 >= pos)
        {
            if (numRows > 0)
            {
                                coords2.SetRow( row2 + numRows );
                if ((size_t)row1 >= pos)
                    coords1.SetRow( row1 + numRows );
            }
            else if (numRows < 0)
            {
                                if ((size_t)row2 >= pos - numRows)
                {
                                        coords2.SetRow( row2 + numRows );
                    if ((size_t)row1 >= pos)
                        coords1.SetRow( wxMax(row1 + numRows, (int)pos) );

                }
                else
                {
                    if ((size_t)row1 >= pos)
                    {
                                                m_blockSelectionTopLeft.RemoveAt(n);
                        m_blockSelectionBottomRight.RemoveAt(n);
                        n--;
                        count--;
                    }
                    else
                        coords2.SetRow( pos );
                }
            }
        }
    }

    count = m_rowSelection.GetCount();
    for ( n = 0; n < count; n++ )
    {
    int  rowOrCol_ = m_rowSelection[n];

      if ((size_t) rowOrCol_ >= pos)
      {
          if ( numRows > 0 )
          {
              m_rowSelection[n] += numRows;
          }
          else if ( numRows < 0 )
          {
              if ((size_t)rowOrCol_ >= (pos - numRows))
                  m_rowSelection[n] += numRows;
              else
              {
                  m_rowSelection.RemoveAt( n );
                  n--;
                  count--;
              }
          }
      }
    }
        
    if ( !m_grid->GetNumberRows() )
        m_colSelection.Clear();
}


void wxGridSelection::UpdateCols( size_t pos, int numCols )
{
    size_t count = m_cellSelection.GetCount();
    size_t n;

    for ( n = 0; n < count; n++ )
    {
        wxGridCellCoords& coords = m_cellSelection[n];
        wxCoord col = coords.GetCol();
        if ((size_t)col >= pos)
        {
            if (numCols > 0)
            {
                                coords.SetCol(col + numCols);
            }
            else if (numCols < 0)
            {
                                if ((size_t)col >= pos - numCols)
                {
                                        coords.SetCol(col + numCols);
                }
                else
                {
                                        m_cellSelection.RemoveAt(n);
                    n--;
                    count--;
                }
            }
        }
    }

    count = m_blockSelectionTopLeft.GetCount();
    for ( n = 0; n < count; n++ )
    {
        wxGridCellCoords& coords1 = m_blockSelectionTopLeft[n];
        wxGridCellCoords& coords2 = m_blockSelectionBottomRight[n];
        wxCoord col1 = coords1.GetCol();
        wxCoord col2 = coords2.GetCol();

        if ((size_t)col2 >= pos)
        {
            if (numCols > 0)
            {
                                coords2.SetCol(col2 + numCols);
                if ((size_t)col1 >= pos)
                    coords1.SetCol(col1 + numCols);
            }
            else if (numCols < 0)
            {
                                if ((size_t)col2 >= pos - numCols)
                {
                                        coords2.SetCol(col2 + numCols);
                    if ( (size_t) col1 >= pos)
                        coords1.SetCol( wxMax(col1 + numCols, (int)pos) );

                }
                else
                {
                    if ((size_t)col1 >= pos)
                    {
                                                m_blockSelectionTopLeft.RemoveAt(n);
                        m_blockSelectionBottomRight.RemoveAt(n);
                        n--;
                        count--;
                    }
                    else
                        coords2.SetCol(pos);
                }
            }
        }
    }

    count = m_colSelection.GetCount();
    for ( n = 0; n < count; n++ )
    {
        int   rowOrCol = m_colSelection[n];

        if ((size_t)rowOrCol >= pos)
        {
            if ( numCols > 0 )
                m_colSelection[n] += numCols;
            else if ( numCols < 0 )
            {
                if ((size_t)rowOrCol >= (pos - numCols))
                    m_colSelection[n] += numCols;
                else
                {
                    m_colSelection.RemoveAt( n );
                    n--;
                    count--;
                }
            }
        }
    }

            if ( !m_grid->GetNumberCols() )
        m_rowSelection.Clear();
}

int wxGridSelection::BlockContain( int topRow1, int leftCol1,
                                   int bottomRow1, int rightCol1,
                                   int topRow2, int leftCol2,
                                   int bottomRow2, int rightCol2 )
{
    if ( topRow1 <= topRow2 && bottomRow2 <= bottomRow1 &&
         leftCol1 <= leftCol2 && rightCol2 <= rightCol1 )
        return 1;
    else if ( topRow2 <= topRow1 && bottomRow1 <= bottomRow2 &&
              leftCol2 <= leftCol1 && rightCol1 <= rightCol2 )
        return -1;

    return 0;
}

#endif
