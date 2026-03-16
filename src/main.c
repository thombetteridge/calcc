#include <stddef.h>
#include <string.h>

#include "cimgui.h"
#include "raylib.h"
#include "rlImGui.h"

#include "calcc.h"

// font
#include "embedded_font.inc"

char input_buffer[2048]   = {0};
char previous_input[2048] = {0};
char output_buffer[2048]  = {0};

int main()
{
   int const screen_width  = 384;
   int const screen_height = 576;

   lex_t lex = {0};
   lx_init(&lex, input_buffer);

   tok_array_t toks;
   tok_array_init(&toks);

   InitWindow(screen_width, screen_height, "Stack Calcc");

   rlImGuiSetup(true);
   ImGuiContext *ctx = igGetCurrentContext();

   ImGuiIO io = ctx->IO;

   ImFontAtlas *atlas = io.Fonts;

   ImFontConfig *cfg;
   cfg                       = ImFontConfig_ImFontConfig();
   cfg->FontDataOwnedByAtlas = false;

   float font_size = 20.0f;

   ImFont *mono = ImFontAtlas_AddFontFromMemoryTTF(
       atlas,
       (void *)JetBrainsMono_Regular_ttf,
       (int)JetBrainsMono_Regular_ttf_len,
       font_size,
       cfg,
       NULL);

   SetTargetFPS(30);
   string_t output = {0};

   bool update = true;

   while (!WindowShouldClose()) {

      BeginDrawing();
      ClearBackground(RAYWHITE);
      rlImGuiBegin();
      igPushFont(mono, font_size);

      igSetNextWindowPos((ImVec2){0, 0}, ImGuiCond_Always, (ImVec2){0, 0});
      igSetNextWindowSize((ImVec2){(float)screen_width, (float)screen_height}, ImGuiCond_Always);
      igBegin("Calculator", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);

      /* output window */
      igBeginChild_Str("Results", (ImVec2){0, screen_height * 0.3819660112501453f}, 0, 0);
      igText("Output: ");
      igPushStyleColor_Vec4(ImGuiCol_FrameBg, (ImVec4){0, 0, 0, 0});
      igPushStyleVar_Vec2(ImGuiStyleVar_FramePadding, (ImVec2){0, 0});

      ImVec2 out_avail;
      igGetContentRegionAvail(&out_avail);
      igBeginChild_Str("OutputText", (ImVec2){out_avail.x, out_avail.y}, false, ImGuiWindowFlags_HorizontalScrollbar);

      if (update) {
         lx_init(&lex, input_buffer);
         lx_to_tokens(&lex, &toks);
         for (size_t i =0; i<toks.len; ++i) {
            tok_t tok = toks.ptr[i];
            printf("Tok:%d, Text:%.*s\n",tok.kind, (int)tok.text.len, tok.text.ptr);
         }
         eval(&toks, output_buffer);
         update = false;
      }

      if (output.len > 0) {
         ImVec2 inner_avail;
         igGetContentRegionAvail(&inner_avail);
         igInputTextMultiline("##output", output_buffer, sizeof(output_buffer),
                              (ImVec2){inner_avail.x, inner_avail.y},
                              ImGuiInputTextFlags_ReadOnly, 0, 0);
      }

      igPopStyleVar(1);
      igPopStyleColor(1);

      igEndChild();
      igEndChild();

      /* input window */

      igBeginChild_Str("Editor", (ImVec2){0, 0}, 0, 0);
      igText("Input: ");
      igPushStyleVar_Vec2(ImGuiStyleVar_FramePadding, (ImVec2){4, 4});
      igPushStyleVar_Vec2(ImGuiStyleVar_ItemSpacing, (ImVec2){4, 4});

      ImVec2 avail;
      igGetContentRegionAvail(&avail);
      float right_margin  = 0.0f;
      float bottom_margin = 2.0f;

      if (igInputTextMultiline("##editor", input_buffer, sizeof(input_buffer),
                               (ImVec2){avail.x - right_margin, avail.y - bottom_margin},
                               ImGuiInputTextFlags_AllowTabInput, 0, 0)) {
         if (strcmp(previous_input, input_buffer) != 0) {
            memcpy(previous_input, input_buffer, sizeof(input_buffer));
            lx_init(&lex, previous_input);
            update = true;
         }
      }

      igPopStyleVar(2);
      igEndChild();
      igEnd();
      igPopFont();

      rlImGuiEnd();
      EndDrawing();
   }
   rlImGuiShutdown();
   CloseWindow();

   // lexer_shutdown(&lexer);
   // eval_shutdown();

   return 0;
}
