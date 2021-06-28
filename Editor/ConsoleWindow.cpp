#include "stdafx.h"

#include "ConsoleWindow.h"
#include "GlobalDef.h"
#include "Primative.h"
#include "Mod.h"
#include "Entity.h"
#include "Node.h"
#include "Directional.h"
#include "Viewport.h"
#include "DebugNew.h"
#include "TransformMod.h"

namespace ToolKit
{
  namespace Editor
  {

    TagArgArray::const_iterator GetTag(String tag, const TagArgArray& tagArgs)
    {
      for (TagArgArray::const_iterator ta = tagArgs.cbegin(); ta != tagArgs.cend(); ta++)
      {
        if (ta->first == tag)
        {
          return ta;
        }
      }

      return tagArgs.end();
    }

    bool TagExist(String tag, const TagArgArray& tagArgs)
    {
      return GetTag(tag, tagArgs) != tagArgs.end();
    }

    void ParseVec(Vec3& vec, TagArgCIt tagIt)
    {
      int maxIndx = glm::min((int)tagIt->second.size(), 3);
      for (int i = 0; i < maxIndx; i++)
      {
        vec[i] = (float)std::atof(tagIt->second[i].c_str());
      }
    }

    // Executors
    void BoolCheck(const TagArgArray& tagArgs, bool* val)
    {
      if (tagArgs.empty())
      {
        return;
      }
      if (tagArgs.front().second.empty())
      {
        return;
      }

      String args = tagArgs.front().second.front();

      if (args == "1")
      {
        *val = true;
      }

      if (args == "0")
      {
        *val = false;
      }
    }

    void ShowPickDebugExec(TagArgArray tagArgs)
    {
      BoolCheck(tagArgs, &g_app->m_showPickingDebug);

      if (!g_app->m_showPickingDebug)
      {
        g_app->m_scene.RemoveEntity(StatePickingBase::m_dbgArrow->m_id);
        StatePickingBase::m_dbgArrow = nullptr;

        g_app->m_scene.RemoveEntity(StatePickingBase::m_dbgFrustum->m_id);
        StatePickingBase::m_dbgFrustum = nullptr;
      }
    }

    void ShowOverlayExec(TagArgArray tagArgs)
    {
      BoolCheck(tagArgs, &g_app->m_showOverlayUI);
    }

    void ShowOverlayAlwaysExec(TagArgArray tagArgs)
    {
      BoolCheck(tagArgs, &g_app->m_showOverlayUIAlways);
    }

    void ShowModTransitionsExec(TagArgArray tagArgs)
    {
      BoolCheck(tagArgs, &g_app->m_showStateTransitionsDebug);
    }

    void TransformInternal(TagArgArray tagArgs, bool set)
    {
      Entity* e = g_app->m_scene.GetCurrentSelection();
      if (e == nullptr)
      {
        return;
      }

      TransformationSpace ts = TransformationSpace::TS_WORLD;
      TagArgArray::const_iterator transformSpaceTag = GetTag("ts", tagArgs);
      if (transformSpaceTag != tagArgs.end())
      {
        if (!transformSpaceTag->second.empty())
        {
          String tsStr = transformSpaceTag->second.front();
          if (tsStr == "world")
          {
            ts = TransformationSpace::TS_WORLD;
          }

          if (tsStr == "parent")
          {
            ts = TransformationSpace::TS_PARENT;
          }

          if (tsStr == "local")
          {
            ts = TransformationSpace::TS_LOCAL;
          }
        }
      }

      ActionManager::GetInstance()->AddAction(new TransformAction(e));
      bool actionApplied = false;

      for (TagArg& tagIt : tagArgs)
      {
        String tag = tagIt.first;
        StringArray& args = tagIt.second;

        if (tag.empty())
        {
          continue;
        }

        if (args.empty())
        {
          continue;
        }

        Vec3 transfrom;
        int maxIndx = glm::min((int)args.size(), 3);
        for (int i = 0; i < maxIndx; i++)
        {
          transfrom[i] = (float)std::atof(args[i].c_str());
        }

        if (tag == "r")
        {
          Quaternion qx = glm::angleAxis(glm::radians(transfrom.x), X_AXIS);
          Quaternion qy = glm::angleAxis(glm::radians(transfrom.y), Y_AXIS);
          Quaternion qz = glm::angleAxis(glm::radians(transfrom.z), Z_AXIS);
          Quaternion q = qz * qy * qx;

          if (set)
          {
            e->m_node->SetOrientation(q, ts);
          }
          else
          {
            e->m_node->Rotate(q, ts);
          }
          actionApplied = true;
        }
        else if (tag == "s")
        {
          if (set)
          {
            e->m_node->SetScale(transfrom);
          }
          else
          {
            e->m_node->Scale(transfrom);
          }
          actionApplied = true;
        }
        else if (tag == "t")
        {
          if (set)
          {
            e->m_node->SetTranslation(transfrom, ts);
          }
          else
          {
            e->m_node->Translate(transfrom, ts);
          }
          actionApplied = true;
        }
      }
      
      if (!actionApplied)
      {
        ActionManager::GetInstance()->RemoveLastAction();
      }
    }

    void SetTransformExec(TagArgArray tagArgs)
    {
      TransformInternal(tagArgs, true);
    }

    void TransformExec(TagArgArray tagArgs)
    {
      TransformInternal(tagArgs, false);
    }

    void SetCameraTransformExec(TagArgArray tagArgs)
    {
      TagArgArray::const_iterator viewportTag = GetTag("v", tagArgs);
      if (viewportTag != tagArgs.end())
      {
        if (viewportTag->second.empty()) // Tag cant be empty.
        {
          return;
        }

        if (Viewport* vp = g_app->GetViewport(viewportTag->second[0]))
        {
          if (Camera* c = vp->m_camera)
          {
            if (viewportTag->second.size() == 2)
            {
              Node* node = c->m_node;
              if (viewportTag->second[1] == "top")
              {
                Quaternion ws = glm::angleAxis(glm::pi<float>(), -Y_AXIS) * glm::angleAxis(glm::half_pi<float>(), X_AXIS) * glm::angleAxis(glm::pi<float>(), Y_AXIS);
                node->SetOrientation(ws, TransformationSpace::TS_WORLD);
                if (c->IsOrtographic())
                {
                  node->SetTranslation(Vec3(0.0f, 500.0f, 0.0f), TransformationSpace::TS_WORLD);
                }
              }

              if (viewportTag->second[1] == "front")
              {
                node->SetOrientation(Quaternion());
                if (c->IsOrtographic())
                {
                  node->SetTranslation(Vec3(0.0f, 0.0f, 500.0f), TransformationSpace::TS_WORLD);
                }
              }

              if (viewportTag->second[1] == "left")
              {
                Quaternion ws = glm::angleAxis(glm::half_pi<float>(), -Y_AXIS);
                node->SetOrientation(ws, TransformationSpace::TS_WORLD);
                if (c->IsOrtographic())
                {
                  node->SetTranslation(Vec3(-500.0f, 0.0f, 0.0f), TransformationSpace::TS_WORLD);
                }
              }
            }

            TagArgArray::const_iterator translateTag = GetTag("t", tagArgs);
            if (translateTag != tagArgs.end())
            {
              Vec3 translate;
              ParseVec(translate, translateTag);
              c->m_node->SetTranslation(translate, TransformationSpace::TS_WORLD);
            }
          }
        }
      }
    }

    void GetTransformExec(TagArgArray tagArgs)
    {
      Entity* e = g_app->m_scene.GetCurrentSelection();
      if (e != nullptr)
      {
        auto PrintTransform = [e](TransformationSpace ts) -> void
        {
          Quaternion q = e->m_node->GetOrientation(ts);
          Vec3 t = e->m_node->GetTranslation(ts);
          Vec3 s = e->m_node->GetScale();

          if (ConsoleWindow* cwnd = g_app->GetConsole())
          {
            String str = "T: " + glm::to_string(t);
            cwnd->AddLog(str);
            str = "R: " + glm::to_string(glm::degrees(glm::eulerAngles(q)));
            cwnd->AddLog(str);
            str = "S: " + glm::to_string(s);
            cwnd->AddLog(str);
          }
        };

        if (tagArgs.empty())
        {
          return;
        }
        if (tagArgs.front().second.empty())
        {
          return;
        }

        String tsStr = tagArgs.front().second.front();
        if (tsStr == "world")
        {
          PrintTransform(TransformationSpace::TS_WORLD);
        }

        if (tsStr == "parent")
        {
          PrintTransform(TransformationSpace::TS_PARENT);
        }

        if (tsStr == "local")
        {
          PrintTransform(TransformationSpace::TS_LOCAL);
        }
      }
    }

    void SetTransformOrientationExec(TagArgArray tagArgs)
    {
      if (tagArgs.empty())
      {
        return;
      }
      if (tagArgs.front().second.empty())
      {
        return;
      }

      String tsStr = tagArgs.front().second.front();
      if (tsStr == "world")
      {
        g_app->m_transformSpace = TransformationSpace::TS_WORLD;
      }

      if (tsStr == "parent")
      {
        g_app->m_transformSpace = TransformationSpace::TS_PARENT;
      }

      if (tsStr == "local")
      {
        g_app->m_transformSpace = TransformationSpace::TS_LOCAL;
      }

      BaseMod* mod = ModManager::GetInstance()->m_modStack.back();
      if (TransformMod* tsm = dynamic_cast<TransformMod*> (mod))
      {
        tsm->m_prevTransformSpace = g_app->m_transformSpace;
        tsm->Signal(TransformMod::m_backToStart);
      }
    }

    void SetImportSlient(TagArgArray tagArgs)
    {
      BoolCheck(tagArgs, &g_app->m_importSlient);
    }

    void SelectByTag(TagArgArray tagArgs)
    {
      if (tagArgs.empty())
      {
        return;
      }
      if (tagArgs.front().second.empty())
      {
        return;
      }

      String args = tagArgs.front().second.front();

      g_app->m_scene.SelectByTag(args);
    }

    void LookAt(TagArgArray tagArgs)
    {
      TagArgArray::const_iterator targetTag = GetTag("t", tagArgs);
      if (targetTag != tagArgs.end())
      {
        if (targetTag->second.empty()) // Tag cant be empty.
        {
          return;
        }

        if (targetTag->second.empty())
        {
          return;
        }

        Vec3 target;
        ParseVec(target, targetTag);
        Viewport* vp = g_app->GetViewport("Perspective");
        if (vp)
        {
          vp->m_camera->LookAt(target);
        }
      }
    }

    void ApplyTransformToMesh(TagArgArray tagArgs)
    {
      // Caviate: A reload is neded since hardware buffers are not updated.
      // After refreshing hardware buffers, transforms of the entity can be set to identity.
      if (Drawable* e = dynamic_cast<Drawable*> (g_app->m_scene.GetCurrentSelection()))
      {
        Mat4 ts = e->m_node->GetTransform(TransformationSpace::TS_WORLD);
        MeshRawPtrArray meshes;
        e->m_mesh->GetAllMeshes(meshes);
        for (Mesh* m : meshes)
        {
          m->ApplyTransform(ts);
        }
      }
      else
      {
        g_app->GetConsole()->AddLog(g_noValidEntity, ConsoleWindow::LogType::Error);
      }
    }

    void SaveMesh(TagArgArray tagArgs)
    {
      if (Drawable* e = dynamic_cast<Drawable*> (g_app->m_scene.GetCurrentSelection()))
      {
        TagArgArray::const_iterator nameTag = GetTag("n", tagArgs);
        String fileName = e->m_mesh->m_file;
        if (fileName.empty())
        {
          fileName = MeshPath(e->m_name + MESH);
        }

        if (nameTag != tagArgs.end())
        {
          fileName = MeshPath(nameTag->second.front() + MESH);
        }

        std::ofstream file;
        file.open(fileName.c_str(), std::ios::out);
        if (file.is_open())
        {
          XmlDocument doc;
          e->m_mesh->Serialize(&doc, nullptr);
          std::string xml;
          rapidxml::print(std::back_inserter(xml), doc, 0);

          file << xml;
          file.close();
          doc.clear();
          g_app->GetConsole()->AddLog("Mesh: " + fileName + " saved.");
        }
      }
      else
      {
        g_app->GetConsole()->AddLog(g_noValidEntity, ConsoleWindow::LogType::Error);
      }
    }

    // ImGui ripoff. Portable helpers.
    static int Stricmp(const char* str1, const char* str2) { int d; while ((d = toupper(*str2) - toupper(*str1)) == 0 && *str1) { str1++; str2++; } return d; }
    static int Strnicmp(const char* str1, const char* str2, int n) { int d = 0; while (n > 0 && (d = toupper(*str2) - toupper(*str1)) == 0 && *str1) { str1++; str2++; n--; } return d; }
    static char* Strdup(const char* str) { size_t len = strlen(str) + 1; void* buf = malloc(len); IM_ASSERT(buf); return (char*)memcpy(buf, (const void*)str, len); }
    static void Strtrim(char* str) { char* str_end = str + strlen(str); while (str_end > str && str_end[-1] == ' ') str_end--; *str_end = 0; }

    ConsoleWindow::ConsoleWindow()
    {
      CreateCommand(g_showPickDebugCmd, ShowPickDebugExec);
      CreateCommand(g_showOverlayUICmd, ShowOverlayExec);
      CreateCommand(g_showOverlayUIAlwaysCmd, ShowOverlayAlwaysExec);
      CreateCommand(g_showModTransitionsCmd, ShowModTransitionsExec);
      CreateCommand(g_setTransformCmd, SetTransformExec);
      CreateCommand(g_setCameraTransformCmd, SetCameraTransformExec);
      CreateCommand(g_transformCmd, TransformExec);
      CreateCommand(g_getTransformCmd, GetTransformExec);
      CreateCommand(g_setTransformOrientationCmd, SetTransformOrientationExec);
      CreateCommand(g_importSlientCmd, SetImportSlient);
      CreateCommand(g_selectByTag, SelectByTag);
      CreateCommand(g_lookAt, LookAt);
      CreateCommand(g_applyTransformToMesh, ApplyTransformToMesh);
      CreateCommand(g_saveMesh, SaveMesh);
    }

    ConsoleWindow::~ConsoleWindow()
    {
    }

    void ConsoleWindow::Show()
    {
      //ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiCond_Once);
      if (ImGui::Begin(g_consoleStr.c_str(), &m_visible))
      {
        HandleStates();

        // Output window.
        const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
        ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), false, ImGuiWindowFlags_HorizontalScrollbar);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1));
        for (size_t i = 0; i < m_items.size(); i++)
        {
          const char* item = m_items[i].c_str();
          if (!m_filter.PassFilter(item))
          {
            continue;
          }

          bool pop_color = false;
          if (strstr(item, g_errorStr.c_str()))
          {
            ImGui::PushStyleColor(ImGuiCol_Text, g_consoleErrorColor);
            pop_color = true;
          }
          else if (strstr(item, g_commandStr.c_str()))
          {
            ImGui::PushStyleColor(ImGuiCol_Text, g_consoleCommandColor);
            pop_color = true;
          }
          else if (strstr(item, g_warningStr.c_str()))
          {
            ImGui::PushStyleColor(ImGuiCol_Text, g_consoleWarningColor);
            pop_color = true;
          }
          else // Than its a memo.
          {
            ImGui::PushStyleColor(ImGuiCol_Text, g_consoleMemoColor);
            pop_color = true;
          }

          ImGui::TextUnformatted(item);
          if (pop_color)
          {
            ImGui::PopStyleColor();
          }
        }

        if (m_scrollToBottom || ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        {
          ImGui::SetScrollHereY(1.0f);
          m_scrollToBottom = false;
        }

        ImGui::PopStyleVar();
        ImGui::EndChild();

        ImGui::Separator();

        ImGui::Text("Command: ");
        ImGui::SameLine();

        // Command window.
        static bool reclaimFocus = false;
        static char inputBuff[256];
        float width = ImGui::GetWindowContentRegionWidth() * 0.4f;

        if (reclaimFocus)
        {
          ImGui::SetKeyboardFocusHere();
        }

        ImGui::PushItemWidth(width);
        if (
          ImGui::InputText
          (
            "##Input",
            inputBuff,
            IM_ARRAYSIZE(inputBuff),
            ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory,
            [](ImGuiInputTextCallbackData* data)->int { return ((ConsoleWindow*)(data->UserData))->TextEditCallback(data); },
            (void*)this
          )
          )
        {
          char* s = inputBuff;
          Strtrim(s);
          if (s[0])
          {
            ExecCommand(s);
          }
          strcpy_s(s, sizeof(inputBuff), "");
          reclaimFocus = true;
        }
        else
        {
          reclaimFocus = false;
        }
        ImGui::PopItemWidth();
        ImGui::SameLine();

        // Search bar.
        ImGui::Text("Filter: ");
        ImGui::SameLine();

        m_filter.Draw("##Filter", width * 0.5f);
        ImGui::SameLine();

        if (ImGui::Button("Clear"))
        {
          m_items.clear();
        }
      }
      ImGui::End();
    }

    Window::Type ConsoleWindow::GetType() const
    {
      return Type::Console;
    }

    void ConsoleWindow::AddLog(const String& log, LogType type)
    {
      String prefixed;
      switch (type)
      {
      case LogType::Error:
        prefixed = g_errorStr + log;
        break;
      case LogType::Warning:
        prefixed = g_warningStr + log;
        break;
      case LogType::Command:
        prefixed = g_commandStr + log;
        break;
      case LogType::Memo:
      default:
        prefixed = g_memoStr + log;
        break;
      }

      m_items.push_back(prefixed);
      m_scrollToBottom = true;
    }

    void ConsoleWindow::AddLog(const String& log, const String& tag)
    {
      String prefixed = "[" + tag + "] " + log;
      m_items.push_back(prefixed);
      m_scrollToBottom = true;
    }

    void ConsoleWindow::ClearLog()
    {
      m_items.clear();
    }

    void ConsoleWindow::ExecCommand(const String& commandLine)
    {
      // Split command and args.
      String cmd;
      TagArgArray tagArgs;
      ParseCommandLine(commandLine, cmd, tagArgs);

      // Insert into history. First find match and delete it so it can be pushed to the back. This isn't trying to be smart or optimal.
      m_historyPos = -1;
      for (int i = (int)m_history.size() - 1; i >= 0; i--)
      {
        if (Stricmp(m_history[i].c_str(), commandLine.c_str()) == 0)
        {
          m_history.erase(m_history.begin() + i);
          break;
        }
      }
      m_history.push_back(commandLine);

      // Process command.
      char buffer[256];
      if (m_commandExecutors.find(cmd) != m_commandExecutors.end())
      {
        AddLog(commandLine, LogType::Command);
        m_commandExecutors[cmd](tagArgs);
      }
      else
      {
        sprintf_s(buffer, sizeof(buffer), "Unknown command: '%s'\n", cmd.c_str());
        AddLog(buffer, LogType::Error);
      }

      m_scrollToBottom = true;
    }

    void SplitPreserveText(const String& s, const String& sep, StringArray& v)
    {
      String arg = s;
      const char spaceSub = (char)26;

      size_t indx = arg.find_first_of('"');
      size_t indx2 = arg.find_first_of('"', indx + 1);
      while (indx != String::npos && indx2 != String::npos)
      {
        std::replace(arg.begin() + indx, arg.begin() + indx2, ' ', spaceSub);
        std::swap(indx, indx2);
        indx2 = arg.find_first_of('"', indx + 1);
      }

      Split(arg, sep, v);

      for (String& val : v)
      {
        val.erase
        (
          std::remove_if(val.begin(), val.end(), [](char c) { return c == '"'; }),
          val.end()
        );

        std::replace(val.begin(), val.end(), spaceSub, ' ');
      }
    }

    void ConsoleWindow::ParseCommandLine(String commandLine, String& command, TagArgArray& tagArgs)
    {
      String args;
      size_t indx = commandLine.find_first_of(" ");
      if (indx != String::npos)
      {
        args = commandLine.substr(indx + 1);
        command = commandLine.substr(0, indx);
      }
      else
      {
        command = commandLine;
        return;
      }

      // Single arg, no tag.
      if (args.find_first_of("--") == String::npos)
      {
        TagArg nullTag;
        StringArray values;
        SplitPreserveText(args, " ", nullTag.second);
        tagArgs.push_back(nullTag);
        return;
      }

      StringArray splits;
      Split(args, "--", splits);

      // Preserve all spaces in text.
      const char spaceSub = (char)26;
      for (String& arg : splits)
      {
        StringArray values;
        SplitPreserveText(arg, " ", values);
        if (values.empty())
        {
          continue;
        }

        String tag;
        tag = values.front();
        pop_front(values);

        if (values.empty())
        {
          continue;
        }

        TagArg input(tag, values);
        tagArgs.push_back(input);
      }
    }

    // Mostly ripoff from the ImGui Console Example.
    int ConsoleWindow::TextEditCallback(ImGuiInputTextCallbackData* data)
    {
      char buffer[256];

      switch (data->EventFlag)
      {
      case ImGuiInputTextFlags_CallbackCompletion:
      {
        // Locate beginning of current word
        const char* word_end = data->Buf + data->CursorPos;
        const char* word_start = word_end;
        while (word_start > data->Buf)
        {
          const char c = word_start[-1];
          if (c == ' ' || c == '\t' || c == ',' || c == ';')
          {
            break;
          }
          word_start--;
        }

        // Build a list of candidates
        StringArray candidates;
        for (size_t i = 0; i < m_commands.size(); i++)
        {
          if (Strnicmp(m_commands[i].c_str(), word_start, (int)(word_end - word_start)) == 0)
          {
            candidates.push_back(m_commands[i]);
          }
        }

        if (candidates.empty())
        {
          // No match
          sprintf_s(buffer, sizeof(buffer), "No match for \"%.*s\"!\n", (int)(word_end - word_start), word_start);
          AddLog(buffer);
        }
        else if (candidates.size() == 1)
        {
          // Single match. Delete the beginning of the word and replace it entirely so we've got nice casing
          data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
          data->InsertChars(data->CursorPos, candidates[0].c_str());
          data->InsertChars(data->CursorPos, " ");
        }
        else
        {
          // Multiple matches. Complete as much as we can, so inputing "C" will complete to "CL" and display "CLEAR" and "CLASSIFY"
          int match_len = (int)(word_end - word_start);
          for (;;)
          {
            int c = 0;
            bool all_candidates_matches = true;
            for (size_t i = 0; i < candidates.size() && all_candidates_matches; i++)
              if (i == 0)
              {
                c = toupper(candidates[i][match_len]);
              }
              else if (c == 0 || c != toupper(candidates[i][match_len]))
              {
                all_candidates_matches = false;
              }
            if (!all_candidates_matches)
            {
              break;
            }
            match_len++;
          }

          if (match_len > 0)
          {
            data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
            data->InsertChars(data->CursorPos, candidates[0].c_str(), candidates[0].c_str() + match_len);
          }

          // List matches
          AddLog("Possible matches:\n");
          for (size_t i = 0; i < candidates.size(); i++)
          {
            sprintf_s(buffer, sizeof(buffer), "- %s\n", candidates[i].c_str());
            AddLog(buffer);
          }
        }

        break;
      }
      case ImGuiInputTextFlags_CallbackHistory:
      {
        const int prev_history_pos = m_historyPos;
        if (data->EventKey == ImGuiKey_UpArrow)
        {
          if (m_historyPos == -1)
          {
            m_historyPos = (int)m_history.size() - 1;
          }
          else if (m_historyPos > 0)
          {
            m_historyPos--;
          }
        }
        else if (data->EventKey == ImGuiKey_DownArrow)
        {
          if (m_historyPos != -1)
          {
            if (++m_historyPos >= m_history.size())
            {
              m_historyPos = -1;
            }
          }
        }

        // A better implementation would preserve the data on the current input line along with cursor position.
        if (prev_history_pos != m_historyPos)
        {
          const char* history_str = (m_historyPos >= 0) ? m_history[m_historyPos].c_str() : "";
          data->DeleteChars(0, data->BufTextLen);
          data->InsertChars(0, history_str);
        }
      }
      }

      return 0;
    }

    void ConsoleWindow::CreateCommand(const String& command, std::function<void(TagArgArray)> executor)
    {
      m_commands.push_back(command);
      m_commandExecutors[command] = executor;
    }

  }
}
