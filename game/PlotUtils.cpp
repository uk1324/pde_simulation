#include "PlotUtils.hpp"
#include <imgui/implot_internal.h>

//bool dragBoundedPoint(int n_id, double* x, double* y, const ImVec4& col) {
//
//    ImGui::PushID("#IMPLOT_DRAG_POINT");
//    IM_ASSERT_USER_ERROR(GImPlot->CurrentPlot != nullptr, "DragPoint() needs to be called between BeginPlot() and EndPlot()!");
//    SetupLock();
//
//    if (!ImHasFlag(flags, ImPlotDragToolFlags_NoFit) && FitThisFrame()) {
//        FitPoint(ImPlotPoint(*x, *y));
//    }
//
//    const bool input = !ImHasFlag(flags, ImPlotDragToolFlags_NoInputs);
//    const bool show_curs = !ImHasFlag(flags, ImPlotDragToolFlags_NoCursors);
//    const bool no_delay = !ImHasFlag(flags, ImPlotDragToolFlags_Delayed);
//    const float grab_half_size = ImMax(DRAG_GRAB_HALF_SIZE, radius);
//    const ImVec4 color = IsColorAuto(col) ? ImGui::GetStyleColorVec4(ImGuiCol_Text) : col;
//    const ImU32 col32 = ImGui::ColorConvertFloat4ToU32(color);
//
//    ImVec2 pos = PlotToPixels(*x, *y, IMPLOT_AUTO, IMPLOT_AUTO);
//    const ImGuiID id = ImGui::GetCurrentWindow()->GetID(n_id);
//    ImRect rect(pos.x - grab_half_size, pos.y - grab_half_size, pos.x + grab_half_size, pos.y + grab_half_size);
//    bool hovered = false, held = false;
//
//    ImGui::KeepAliveID(id);
//    if (input) {
//        bool clicked = ImGui::ButtonBehavior(rect, id, &hovered, &held);
//        if (out_clicked) *out_clicked = clicked;
//        if (out_hovered) *out_hovered = hovered;
//        if (out_held)    *out_held = held;
//    }
//
//    bool modified = false;
//    if (held && ImGui::IsMouseDragging(0)) {
//        *x = ImPlot::GetPlotMousePos(IMPLOT_AUTO, IMPLOT_AUTO).x;
//        *y = ImPlot::GetPlotMousePos(IMPLOT_AUTO, IMPLOT_AUTO).y;
//        modified = true;
//    }
//
//    PushPlotClipRect();
//    ImDrawList& DrawList = *GetPlotDrawList();
//    if ((hovered || held) && show_curs)
//        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
//    if (modified && no_delay)
//        pos = PlotToPixels(*x, *y, IMPLOT_AUTO, IMPLOT_AUTO);
//    DrawList.AddCircleFilled(pos, radius, col32);
//    PopPlotClipRect();
//
//    ImGui::PopID();
//    return modified;
//}