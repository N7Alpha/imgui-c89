#include "input.cpp"

int main()
{
    ImGui::Vec2 value(2.0f, 3.0f);
    bool return_value_preserved = ImGui::ReturnCleanup() == 0;
    bool return_cleanup_ran = ImGui::ReadDestructorState() == 7;
    return ImGui::ScaledSum(value) == 5.0f && ImGui::SumTo(5) == 10 &&
                   GlobalDouble(4) == 8 && ImGui::CleanupOrder() == 291 &&
                   return_value_preserved && return_cleanup_ran &&
                   ImGui::LoopCleanup() == 12 && ImGui::StackScratch(4) == 10 &&
                   ImGui::DiscardedReference() == 9.0f
               ? 0
               : 1;
}
