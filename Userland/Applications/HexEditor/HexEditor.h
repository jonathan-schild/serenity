/*
 * Copyright (c) 2018-2020, Andreas Kling <kling@serenityos.org>
 * Copyright (c) 2021, Mustafa Quraish <mustafa@serenityos.org>
 * Copyright (c) 2022, the SerenityOS developers.
 * Copyright (c) 2022, Timothy Slater <tslater2006@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#include "HexDocument.h"
#include "SearchResultsModel.h"
#include "Selection.h"
#include <AK/ByteBuffer.h>
#include <AK/Function.h>
#include <AK/HashMap.h>
#include <AK/StdLibExtras.h>
#include <LibCore/Timer.h>
#include <LibGUI/AbstractScrollableWidget.h>
#include <LibGUI/UndoStack.h>
#include <LibGfx/Font/Font.h>
#include <LibGfx/TextAlignment.h>

namespace HexEditor {

class HexEditor : public GUI::AbstractScrollableWidget {
    C_OBJECT(HexEditor)
public:
    enum EditMode {
        Hex,
        Text
    };

    virtual ~HexEditor() override = default;

    size_t buffer_size() const { return m_document->size(); }
    ErrorOr<void> open_new_file(size_t size);
    void open_file(NonnullOwnPtr<Core::File> file);
    ErrorOr<void> fill_selection(u8 fill_byte);
    Optional<u8> get_byte(size_t position);
    ByteBuffer get_selected_bytes();
    ErrorOr<void> save_as(NonnullOwnPtr<Core::File>);
    ErrorOr<void> save();

    bool undo();
    bool redo();
    GUI::UndoStack& undo_stack();

    void select_all();
    Selection const& selection() const { return m_selection; }
    bool has_selection() const { return !m_selection.is_empty() && m_document->size() > 0; }
    size_t selection_start_offset() const { return m_selection.start; }
    bool copy_selected_text_to_clipboard();
    bool copy_selected_hex_to_clipboard();
    bool copy_selected_hex_to_clipboard_as_c_code();

    size_t bytes_per_row() const { return m_bytes_per_row; }
    void set_bytes_per_row(size_t);

    void set_position(size_t position);
    void set_selection(size_t position, size_t length);
    void highlight(size_t start, size_t end);
    Optional<size_t> find(ByteBuffer& needle, size_t start = 0);
    Optional<size_t> find_and_highlight(ByteBuffer& needle, size_t start = 0);
    Vector<Match> find_all(ByteBuffer& needle, size_t start = 0);
    Vector<Match> find_all_strings(size_t min_length = 4);
    Function<void(size_t position, EditMode, Selection)> on_status_change;
    Function<void(bool is_document_dirty)> on_change;

    void show_create_annotation_dialog();
    void show_edit_annotation_dialog(Annotation&);
    void show_delete_annotation_dialog(Annotation&);

    HexDocument& document() { return *m_document; }

protected:
    HexEditor();

    virtual void paint_event(GUI::PaintEvent&) override;
    virtual void mousedown_event(GUI::MouseEvent&) override;
    virtual void mouseup_event(GUI::MouseEvent&) override;
    virtual void mousemove_event(GUI::MouseEvent&) override;
    virtual void keydown_event(GUI::KeyEvent&) override;
    virtual void context_menu_event(GUI::ContextMenuEvent&) override;

private:
    size_t m_line_spacing { 4 };
    size_t m_content_length { 0 };
    size_t m_bytes_per_row { 16 };
    bool m_in_drag_select { false };
    Selection m_selection;
    size_t m_position { 0 };
    bool m_cursor_at_low_nibble { false };
    EditMode m_edit_mode { Hex };
    NonnullOwnPtr<HexDocument> m_document;
    GUI::UndoStack m_undo_stack;
    Optional<Annotation&> m_hovered_annotation;

    RefPtr<GUI::Menu> m_context_menu;
    RefPtr<GUI::Action> m_add_annotation_action;
    RefPtr<GUI::Action> m_edit_annotation_action;
    RefPtr<GUI::Action> m_delete_annotation_action;

    static constexpr int m_address_bar_width = 90;
    static constexpr int m_padding = 5;

    void scroll_position_into_view(size_t position);

    size_t total_rows() const { return ceil_div(m_content_length, m_bytes_per_row); }
    size_t line_height() const { return font().pixel_size_rounded_up() + m_line_spacing; }
    size_t character_width() const { return font().glyph_width('W'); }
    size_t cell_width() const { return character_width() * 3; }
    size_t offset_margin_width() const { return 80; }

    struct OffsetData {
        size_t offset;
        EditMode panel;
    };
    Optional<OffsetData> offset_at(Gfx::IntPoint position) const;

    ErrorOr<void> hex_mode_keydown_event(GUI::KeyEvent&);
    ErrorOr<void> text_mode_keydown_event(GUI::KeyEvent&);

    void set_content_length(size_t); // I might make this public if I add fetching data on demand.
    void update_status();
    void did_change();
    ErrorOr<void> did_complete_action(size_t position, u8 old_value, u8 new_value);
    ErrorOr<void> did_complete_action(size_t position, ByteBuffer&& old_values, ByteBuffer&& new_values);
};

}
