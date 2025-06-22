# Refactoring charproc.c

## Overview

The `charproc.c` file is currently 15,004 lines (400K) and contains multiple distinct functional areas that could be separated into smaller, more manageable modules. This document outlines a strategy for splitting this large file while maintaining existing functionality and interfaces.

## Current State

- **File**: `charproc.c`
- **Size**: 15,004 lines, 400K
- **Functionality**: Core terminal character processing, VT parsing, display management, and terminal state handling

## Major Functional Areas to Extract

### 1. **VT Parser Core** (`vt_parser.c` - ~3,400 lines)
- **Main function**: `doparsing()` (lines 3055-6445) - The massive VT terminal parser
- **Key functions**: `VTparse()`, `init_parser()`, `illegal_parse()`
- **Responsibility**: Core VT escape sequence parsing and state machine
- **Dependencies**: Minimal - mostly self-contained parser logic

### 2. **SGR & Color Management** (`sgr_colors.c` - ~300 lines) 
- **Key functions**: 
  - `SGR_Foreground()`, `SGR_Background()`
  - `setExtendedFG()`, `setExtendedBG()`, `setExtendedColors()`
  - `reset_SGR_Foreground()`, `reset_SGR_Background()`, `reset_SGR_Colors()`
- **Responsibility**: Text styling: bold, italic, underline, colors
- **Clear boundaries**: Well-defined color and rendering attribute management

### 3. **Character Set Management** (`charsets.c` - ~400 lines)
- **Key functions**:
  - `initCharset()`, `saveCharsets()`, `restoreCharsets()`, `resetCharsets()`
  - `current_charset()`, `select_charset()`, `xtermDecodeSCS()`
  - `encode_scs()`, `is_96charset()`
- **Responsibility**: National character sets, G-sets, charset switching
- **Clear boundaries**: Character encoding and charset selection logic

### 4. **Terminal Modes** (`terminal_modes.c` - ~800 lines)
- **Key functions**:
  - `ansi_modes()`, `dpmodes()`, `savemodes()`, `restoremodes()`
  - `set_ansi_conformance()`, `set_vtXX_level()`
  - Mode setting/clearing bit manipulation functions
- **Responsibility**: ANSI/DEC mode handling, terminal state persistence
- **Clear boundaries**: Mode flag management is largely independent

### 5. **Window Operations** (`window_ops.c` - ~600 lines)
- **Key functions**:
  - `window_ops()` - handles window manipulation commands
  - Window label functions: `get_icon_label()`, `get_window_label()`, `report_win_label()`
  - Property handling: `property_to_string()`
- **Responsibility**: Window resizing, title/icon management, window queries
- **Clear boundaries**: X11 window system interactions

### 6. **Cursor & Display** (`cursor_display.c` - ~700 lines)
- **Key functions**:
  - `ShowCursor()`, `HideCursor()` (large functions ~400+ lines combined)
  - `WrapLine()`, `updateCursor()`
  - Blinking functions: `StartBlinking()`, `StopBlinking()`, `HandleBlinking()`
- **Responsibility**: Cursor rendering, positioning, blinking behavior
- **Clear boundaries**: Visual cursor management

### 7. **Text Rendering** (`text_render.c` - ~500 lines)
- **Key functions**:
  - `dotext()` - main text rendering function
  - `v_write()` - PTY output handling
  - Text measurement and formatting functions
- **Responsibility**: Character output, line wrapping, text rendering
- **Dependencies**: Interacts with charset and SGR modules

### 8. **Margins & Scrolling** (`margins_scroll.c` - ~300 lines)
- **Key functions**:
  - `set_tb_margins()`, `set_lr_margins()`, `resetMargins()`
  - `set_max_col()`, `set_max_row()`
  - Scrolling region management
- **Responsibility**: Terminal margins, scrolling regions
- **Clear boundaries**: Geometric layout management

### 9. **Status Line Management** (`status_line.c` - ~400 lines) [OPT_STATUS_LINE]
- **Key functions**:
  - `update_status_line()`, `clear_status_line()`, `show_indicator_status()`
  - `handle_DECSASD()`, `handle_DECSSDT()`
- **Responsibility**: DEC-style status line display
- **Clear boundaries**: Optional feature with clear API

### 10. **Parameter Parsing** (`param_parser.c` - ~400 lines)
- **Key functions**:
  - `init_params()`, `parse_extended_colors()`, `get_subparam()`
  - `optional_param()`, `use_default_value()`
  - Parameter validation and extraction functions
- **Responsibility**: ESC sequence parameter parsing
- **Clear boundaries**: Utility functions for parser

### 11. **Initialization & Resources** (`charproc_init.c` - ~500 lines)
- **Key functions**:
  - Resource table definitions, initialization functions
  - Action table definitions
  - Translation table management
- **Responsibility**: X11 resources, action bindings, initialization
- **Clear boundaries**: Startup and configuration

## Proposed Directory Structure

```
charproc/
├── vt_parser.c         # Core VT parsing state machine (~3400 lines)
├── sgr_colors.c        # SGR and color management (~300 lines)
├── charsets.c          # Character set management (~400 lines)
├── terminal_modes.c    # Mode setting/clearing (~800 lines)
├── window_ops.c        # Window operations (~600 lines)
├── cursor_display.c    # Cursor and display management (~700 lines)
├── text_render.c       # Text rendering and output (~500 lines)
├── margins_scroll.c    # Margins and scrolling (~300 lines)
├── status_line.c       # Status line management (~400 lines) [optional]
├── param_parser.c      # Parameter parsing utilities (~400 lines)
├── charproc_init.c     # Initialization and resources (~500 lines)
└── charproc.h          # Common definitions and interfaces
```

## Benefits of This Refactoring

1. **Maintainability**: Each module has a clear, focused responsibility
2. **Testability**: Individual modules can be unit tested in isolation
3. **Debugging**: Issues can be localized to specific functional areas
4. **Code Review**: Smaller files are easier to review and understand
5. **Parallel Development**: Different developers can work on different modules
6. **Conditional Compilation**: Optional features (like status line) are clearly separated

## Implementation Strategy

### Phase 1: Preparation
1. **Create comprehensive tests** for existing functionality
2. **Document current interfaces** and dependencies
3. **Set up build system** to handle multiple source files
4. **Create shared header** (`charproc.h`) with common definitions

### Phase 2: Extract Independent Modules (Low Risk)
1. **Start with `charsets.c`**: Character set management is well-isolated
2. **Extract `param_parser.c`**: Parameter parsing utilities
3. **Move `status_line.c`**: Optional feature with clear boundaries
4. **Extract `charproc_init.c`**: Initialization and resource management

### Phase 3: Extract Functional Modules (Medium Risk)
5. **Extract `sgr_colors.c`**: Color and styling management
6. **Move `margins_scroll.c`**: Margin and scrolling region handling
7. **Extract `window_ops.c`**: Window manipulation commands
8. **Move `terminal_modes.c`**: Mode setting and clearing

### Phase 4: Extract Core Display Modules (Higher Risk)
9. **Extract `cursor_display.c`**: Cursor rendering and management
10. **Move `text_render.c`**: Text rendering and output
11. **Extract `vt_parser.c`**: Core VT parsing (most complex)

### Phase 5: Integration and Cleanup
12. **Optimize interfaces** between modules
13. **Remove duplicate code** and consolidate utilities
14. **Update documentation** and build system
15. **Performance testing** and optimization

## Key Considerations

### Dependencies Management
- The massive `doparsing()` function touches almost every aspect of terminal behavior
- This function should remain as a "dispatcher" while delegating specific operations to appropriate modules
- Maintain existing function signatures during transition to avoid breaking external code

### Interface Preservation
- Create compatibility layer during transition
- Maintain all existing public interfaces
- Preserve conditional compilation structure (`#ifdef` blocks)
- Keep all resource definitions and action bindings intact

### Testing Strategy
- Test each module extraction individually
- Maintain full regression test suite
- Use feature flags to enable/disable new modular code during development
- Compare behavior before and after each phase

### Build System Updates
- Update Makefile to compile multiple source files
- Ensure proper dependency tracking
- Maintain compatibility with existing build configurations
- Add module-specific compilation flags if needed

## Potential Challenges

1. **Interdependencies**: Many functions share global state and call each other
2. **Massive `doparsing()` function**: Central parser touches almost everything
3. **Shared data structures**: Terminal state is accessed throughout the code
4. **Conditional compilation**: Many `#ifdef` blocks span multiple functional areas
5. **Performance**: Function call overhead between modules

## Success Metrics

- [ ] All existing tests pass after each phase
- [ ] No regression in terminal functionality
- [ ] Build time remains reasonable
- [ ] Runtime performance maintained
- [ ] Code becomes more maintainable (subjective but measurable through developer feedback)
- [ ] Individual modules can be unit tested
- [ ] New features can be added more easily

## Timeline Estimate

- **Phase 1 (Preparation)**: 2-3 weeks
- **Phase 2 (Independent modules)**: 3-4 weeks  
- **Phase 3 (Functional modules)**: 4-5 weeks
- **Phase 4 (Core display modules)**: 6-8 weeks
- **Phase 5 (Integration)**: 2-3 weeks

**Total estimated time**: 17-23 weeks (4-6 months)

This refactoring should be done incrementally with extensive testing at each step to ensure no functionality is lost during the process.