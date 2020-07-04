/* This file is part of Zutty.
 * Copyright (C) 2020 Tom Szilagyi
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * See the file LICENSE for the full license.
 */

#pragma once

#include "charvdev.h"

#include <cstdint>
#include <functional>
#include <memory>

namespace zutty {

   class Vterm
   {
   public:
      Vterm () = default;

      Vterm (uint16_t glyphPx, uint16_t glyphPy,
             uint16_t winPx, uint16_t winPy,
             uint16_t borderPx, int ptyFd);

      ~Vterm () = default;

      void setRefreshHandler (
         const std::function <void (const Vterm&)>& refreshHandler);

      void resize (uint16_t winPx, uint16_t winPy);

      void copyCells (CharVdev::Cell * const dest);

      enum class VtKey
      {
         NONE,
         Return, Backspace,
         Insert, Delete, Home, End, Up, Down, Left, Right,
         PageUp, PageDown,
         F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
         F13, F14, F15, F16, F17, F18, F19, F20,
         KP_F1, KP_F2, KP_F3, KP_F4,
         KP_Plus, KP_Minus, KP_Comma, KP_Dot, KP_Enter,
         KP_0, KP_1, KP_2, KP_3, KP_4, KP_5, KP_6, KP_7, KP_8, KP_9
      };

      // mapping of a certain VtKey to a sequence of input characters
      struct InputSpec
      {
         VtKey key;
         const char * input;
      };

      int writePty (uint8_t ch);
      int writePty (const char* cstr);
      int writePty (VtKey key);

      void readPty ();

      uint64_t seqNo = 0; // update counter
      bool exit = false;

      uint16_t winPx;
      uint16_t winPy;
      uint16_t nCols;
      uint16_t nRows;

   private:
      void processInput (unsigned char* input, int size);

      // table entry for deciding which set of InputSpecs to use
      struct InputSpecTable
      {
         std::function <bool ()> predicate;
         const InputSpec * specs = nullptr;
         bool visited = false;
      };

      InputSpecTable * getInputSpecTable ();
      void resetInputSpecTable ();
      const InputSpec * selectInputSpecs ();
      const InputSpec & getInputSpec (VtKey key);

      CharVdev::Cell& cell (uint32_t idx);
      CharVdev::Cell& cell (uint16_t row, uint16_t col);

      void debugStop ();
      void unhandledInput (char ch);
      void traceNormalInput ();
      void resetTerminal ();
      void resetAttrs ();
      void resetScreen ();
      void fillScreen (uint16_t ch);
      void linearizeCellStorage ();

      enum class InputState: uint8_t
      {
         Normal,
         Escape,
         Esc_Hash,
         Esc_Pct,
         SelectCharset,
         CSI,
         CSI_priv,
         CSI_Quote,
         CSI_Bang,
         CSI_SPC,
         CSI_GT,
         DCS,
         DCS_Esc,
         OSC,
         OSC_Esc
      };

      void setState (InputState inputState);

      uint32_t getIdx (uint16_t pY, uint16_t pX);
      uint32_t setCur ();
      uint32_t startOfThisLine ();
      uint32_t startOfNextLine ();
      void eraseRange (uint32_t start, uint32_t end);
      void eraseRow (uint16_t pY);
      void copyRow (uint16_t dstY, uint16_t srcY);
      void insertLines (uint16_t count);
      void deleteLines (uint16_t count);

      void normalizePosition ();
      void advancePosition ();
      void showCursor ();
      void hideCursor ();
      void inputGraphicChar (unsigned char ch);
      void placeGraphicChar ();
      void jumpToNextTabStop ();
      void setFgFromPalIx ();
      void setBgFromPalIx ();

      // DEC control sequence handlers, prefixed with input state
      void inp_LF ();        // Line Feed
      void inp_CR ();        // Carriage Return
      void inp_HT ();        // Horizontal Tab

      void esc_DCS (unsigned char fin); // Designate Character Set
      void esc_IND ();       // Index
      void esc_RI ();        // Reverse Index
      void esc_NEL ();       // Next Line
      void esc_BI ();        // Back Index
      void esc_FI ();        // Forward Index
      void esc_HTS ();       // Horizontal Tab Set
      void csi_SCOSC ();     // Save Cursor Position
      void csi_SCORC ();     // Restore Cursor Position
      void esc_DECSC ();     // Save Cursor and Attributes
      void esc_DECRC ();     // Restore Cursor and Attributes
      void esc_RIS ();       // Reset to Initial State

      void csi_CUU ();       // Cursor Up
      void csi_CUD ();       // Cursor Down
      void csi_CUF ();       // Cursor Forward
      void csi_CUB ();       // Cursor Backward
      void csi_CNL ();       // Cursor Next Line
      void csi_CPL ();       // Cursor Previous Line
      void csi_CHA ();       // Cursor Character Absolute
      void csi_HPA ();       // Character Position Absolute
      void csi_HPR ();       // Character Position Relative
      void csi_VPA ();       // Line Position Absolute
      void csi_VPR ();       // Line Position Relative
      void csi_CUP ();       // Cursor Position a.k.a. HVP
      void csi_SU ();        // Pan Down / Scroll Up
      void csi_SD ();        // Pan Up / Scroll Down
      void csi_CHT ();       // Character Tabulation
      void csi_CBT ();       // Character Backwards Tabulation
      void csi_REP ();       // Repeat last graphic character

      void csi_ED ();        // Erase in Display
      void csi_EL ();        // Erase in Line
      void csi_IL ();        // Insert Line
      void csi_DL ();        // Delete Line
      void csi_ICH ();       // Insert Characters
      void csi_DCH ();       // Delete Characters
      void csi_ECH ();       // Erase Characters

      void csi_STBM ();      // Set Top and Bottom Margins
      void csi_TBC ();       // Tabulation Clear

      void csi_SM ();        // Set Mode
      void csi_RM ();        // Reset Mode
      void csi_privSM ();    // Set Mode (private)
      void csi_privRM ();    // Reset Mode (private)
      void csi_SGR ();       // Select Graphic Rendition

      void csi_ecma48_SL (); // Shift Left
      void csi_ecma48_SR (); // Shift Right

      void csi_priDA ();     // Device Attributes (Primary)
      void csi_secDA ();     // Device Attributes (Secondary)
      void csi_DSR ();       // Device State Report
      void esch_DECALN ();   // DEC Alignment Pattern Generator
      void handle_DCS ();    // Device Control String
      void handle_OSC ();    // Operating System Command
      void csiq_DECSCL ();   // DEC Set Compatibility Level

      uint16_t borderPx;
      uint16_t glyphPx;
      uint16_t glyphPy;
      int ptyFd;

      std::function <void (const Vterm&)> refreshVideo;

      // Cell storage, display and input state

      std::shared_ptr <CharVdev::Cell> cells;
      uint32_t cur;           // current screen position (abs. offset in cells)
      uint32_t scrollHead;    // scrolling area start of top row (abs. offset)
      uint16_t marginTop;     // current margin top (number of rows above)
      uint16_t marginBottom;  // current margin bottom (number of rows above + 1)
      uint16_t posX = 0;      // current cursor horizontal position (on-screen)
      uint16_t posY = 0;      // current cursor vertical position (on-screen)
      CharVdev::Cell attrs;   // prototype cell with current attributes
      CharVdev::Color* fg = &attrs.fg;
      CharVdev::Color* bg = &attrs.bg;
      CharVdev::Color palette256 [256];
      constexpr const static int defaultFgPalIx = 15;
      constexpr const static int defaultBgPalIx = 0;
      int fgPalIx = defaultFgPalIx;
      int bgPalIx = defaultBgPalIx;

      unsigned char inputBuf [4096];
      int readPos = 0;
      int lastEscBegin = 0;
      int lastNormalBegin = 0;
      int lastStopPos = 0;

      InputState inputState = InputState::Normal;
      constexpr const static size_t maxEscOps = 16;
      uint32_t inputOps [maxEscOps];
      size_t nInputOps = 0;
      uint16_t unicode_cp = 0;
      uint8_t utf8_rem = 0;
      std::vector <unsigned char> argBuf;
      unsigned char scsDst;  // Select charset / destination designator
      unsigned char scsMod;  // Select charset / selector (intermediate)

      // Terminal state - N.B.: keep resetTerminal () in sync with this!

      bool showCursorMode = true;
      bool altScreenBufferMode = false;
      bool autoWrapMode = true;
      bool autoNewlineMode = false;
      bool insertMode = false;
      bool bkspSendsDel = true;
      bool localEcho = false;

      std::vector <uint16_t> tabStops;

      enum class CompatibilityLevel: uint8_t
      { VT52, VT100, VT400  };
      CompatibilityLevel compatLevel = CompatibilityLevel::VT400;

      enum class CursorKeyMode: uint8_t
      { ANSI, Application };
      CursorKeyMode cursorKeyMode = CursorKeyMode::ANSI;

      enum class NumpadMode: uint8_t
      { Numeric, Application };
      NumpadMode numpadMode = NumpadMode::Numeric;

      enum class OriginMode: uint8_t
      { Absolute, ScrollingRegion };
      OriginMode originMode = OriginMode::Absolute;

      enum class Charset: uint8_t // sync w/charCodes definition!
      { UTF8, DecSpec, DecSuppl, DecUserPref, DecTechn, IsoLatin1, IsoUK };

      struct CharsetState
      {
         Charset g[4] =
         { Charset::UTF8, Charset::UTF8, Charset::UTF8, Charset::UTF8 };

         // Locking shift states (index into g[]):
         uint8_t gl = 0; // G0 in GL
         uint8_t gr = 2; // G2 in GR

         // Single shift state (0 if none active):
         // 0 - not active; 2: G2 in GL; 3: G3 in GL
         uint8_t ss = 0;
      };
      CharsetState charsetState;

      // address with Charset - 1; point to array of 96 unicode points:
      static const uint16_t* charCodes [];

      struct SavedCursor_SCO
      {
         bool isSet = false;
         uint16_t posX = 0;
         uint16_t posY = 0;
      };
      struct SavedCursor_DEC: SavedCursor_SCO
      {
         CharVdev::Cell attrs;
         bool autoWrapMode = true;
         OriginMode originMode = OriginMode::Absolute;
         CharsetState charsetState = CharsetState {};
         // NYI: selective erase mode
      };
      SavedCursor_SCO savedCursor_SCO;
      SavedCursor_DEC savedCursor_DEC;
   };

} // namespace zutty

#include "vterm.icc"