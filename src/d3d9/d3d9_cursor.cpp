#include "d3d9_cursor.h"
#include "d3d9_util.h"

#include <utility>

namespace dxvk {

  void D3D9Cursor::ResetCursor() {
    ShowCursor(FALSE);

    if (m_hCursor != nullptr) {
      ResetHardwareCursor();
    } else if (m_sCursor.Bitmap != nullptr) {
      ResetSoftwareCursor();
    }
  }

  
  void D3D9Cursor::ResetHardwareCursor() {
    ::DestroyCursor(m_hCursor);
    m_hCursor = nullptr;
  }


  void D3D9Cursor::ResetSoftwareCursor() {
      m_sCursor.Bitmap = nullptr;
      m_sCursor.XHotSpot = 0;
      m_sCursor.YHotSpot = 0;
      m_sCursor.X = 0.0f;
      m_sCursor.Y = 0.0f;
  }


  void D3D9Cursor::UpdateCursor(int X, int Y) {
    POINT currentPos = { };
    if (::GetCursorPos(&currentPos) && currentPos == POINT{ X, Y })
        return;

    ::SetCursorPos(X, Y);
  }


  void D3D9Cursor::RefreshSoftwareCursorPosition() {
    POINT currentPos = { };
    ::GetCursorPos(&currentPos);

    m_sCursor.X = static_cast<float>(currentPos.x) - static_cast<float>(m_sCursor.XHotSpot);
    m_sCursor.Y = static_cast<float>(currentPos.y) - static_cast<float>(m_sCursor.YHotSpot);
  }


  BOOL D3D9Cursor::ShowCursor(BOOL bShow) {
    // Cursor visibility remains unchanged (typically FALSE) if the cursor isn't set.
    if (unlikely(m_hCursor == nullptr && m_sCursor.Bitmap == nullptr))
      return m_visible;

    if (m_hCursor != nullptr)
      ::SetCursor(bShow ? m_hCursor : nullptr);

    return std::exchange(m_visible, bShow);
  }


  HRESULT D3D9Cursor::SetHardwareCursor(UINT XHotSpot, UINT YHotSpot, const CursorBitmap& bitmap) {
    if (m_sCursor.Bitmap != nullptr)
      ResetSoftwareCursor();

    CursorMask mask;
    std::memset(mask, ~0, sizeof(mask));

    ICONINFO info;
    info.fIcon    = FALSE;
    info.xHotspot = XHotSpot;
    info.yHotspot = YHotSpot;
    info.hbmMask  = ::CreateBitmap(HardwareCursorWidth, HardwareCursorHeight, 1, 1,  &mask[0]);
    info.hbmColor = ::CreateBitmap(HardwareCursorWidth, HardwareCursorHeight, 1, 32, &bitmap[0]);

    if (m_hCursor != nullptr)
      ::DestroyCursor(m_hCursor);

    m_hCursor = ::CreateIconIndirect(&info);

    ::DeleteObject(info.hbmMask);
    ::DeleteObject(info.hbmColor);

    ShowCursor(m_visible);

    return D3D_OK;
  }

  HRESULT D3D9Cursor::SetSoftwareCursor(UINT XHotSpot, UINT YHotSpot, Com<IDirect3DTexture9> pCursorBitmap) {
    // Make sure to hide the win32 cursor
    ::SetCursor(nullptr);

    if (m_hCursor != nullptr)
      ResetHardwareCursor();

    m_sCursor.Bitmap    = pCursorBitmap;
    m_sCursor.XHotSpot  = XHotSpot;
    m_sCursor.YHotSpot  = YHotSpot;

    ShowCursor(m_visible);

    return D3D_OK;
  }

}