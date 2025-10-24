#include <stddef.h>

#include "cimgui.h"
#include "gc/gc.h"
#include "raylib.h"
#include "rlImGui.h"

#include "eval.h"
#include "gcstring.h"
#include "lexer.h"

char input_buffer[2048];

int main() {
   GC_INIT();

   int const screen_width  = 400;
   int const screen_height = 600;

   Lexer lexer = { 0 };
   lexer_init(&lexer);

   InitWindow(screen_width, screen_height, "test");
   rlImGuiSetup(true);

   SetTargetFPS(30);

   while (!WindowShouldClose()) {
      BeginDrawing();
      ClearBackground(RAYWHITE);
      rlImGuiBegin();

      igSetNextWindowPos((ImVec2) { 0, 0 }, ImGuiCond_Always, (ImVec2) { 0, 0 });
      igSetNextWindowSize((ImVec2) { (float)screen_width, (float)screen_height }, ImGuiCond_Always);
      igBegin("Calculator", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);

      /* output window */
      igBeginChild_Str("Results", (ImVec2) { 0, screen_height * 0.3819660112501453f }, 0, 0);
      igText("Output: ");
      igPushStyleColor_Vec4(ImGuiCol_FrameBg, (ImVec4) { 0, 0, 0, 0 });
      igPushStyleVar_Vec2(ImGuiStyleVar_FramePadding, (ImVec2) { 0, 0 });

      ImVec2 out_avail;
      igGetContentRegionAvail(&out_avail);
      igBeginChild_Str("OutputText", (ImVec2) { out_avail.x, out_avail.y }, false, ImGuiWindowFlags_HorizontalScrollbar);

      GC_String output;
      output = run_calculator(&lexer);
      if (output.len > 0) {
         ImVec2 inner_avail;
         igGetContentRegionAvail(&inner_avail);
         igInputTextMultiline("##output", output.data, output.len,
             (ImVec2) { inner_avail.x, inner_avail.y },
             ImGuiInputTextFlags_ReadOnly, 0, 0);
      }

      igPopStyleVar(1);
      igPopStyleColor(1);

      igEndChild();
      igEndChild();

      /* input window */

      igBeginChild_Str("Editor", (ImVec2) { 0, 0 }, 0, 0);
      igText("Input: ");
      igPushStyleVar_Vec2(ImGuiStyleVar_FramePadding, (ImVec2) { 4, 4 });
      igPushStyleVar_Vec2(ImGuiStyleVar_ItemSpacing, (ImVec2) { 4, 4 });

      ImVec2 avail;
      igGetContentRegionAvail(&avail);
      float right_margin  = 0.0f;
      float bottom_margin = 2.0f;

      igInputTextMultiline("##editor", input_buffer, sizeof(input_buffer),
          (ImVec2) { avail.x - right_margin, avail.y - bottom_margin },
          ImGuiInputTextFlags_AllowTabInput, 0, 0);

      lexer_feed(&lexer, input_buffer, (uint)strlen(input_buffer));

      igPopStyleVar(2);
      igEndChild();
      igEnd();

      rlImGuiEnd();
      EndDrawing();
   }
   rlImGuiShutdown();
   CloseWindow();

   GC_gcollect();
   printf("Heap size: %lu bytes\n", (unsigned long)GC_get_heap_size());
   printf("Bytes allocated since last GC: %lu\n", (unsigned long)GC_get_bytes_since_gc());
   printf("Bytes reclaimed: %lu\n", (unsigned long)GC_get_free_bytes());

   return 0;
}
