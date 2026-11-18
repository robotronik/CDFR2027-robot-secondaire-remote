#include <cassert>
#include "teleop.hpp"
static bool eq(Pose a, Pose b){ return a.x==b.x && a.y==b.y && a.theta==b.theta; }
int main(){
    assert(eq(next_target({0,0,0},'F'),   {100,0,0}));
    assert(eq(next_target({0,0,90},'F'),  {0,100,90}));      // cos 90° rounds to 0
    assert(eq(next_target({0,0,0},'B'),   {-100,0,0}));
    assert(eq(next_target(next_target({0,0,0},'F'),'F'), {200,0,0}));  // two_forwards_accumulate
    assert(eq(next_target({0,0,0},'L'),   {0,0,15}));
    assert(eq(next_target({0,0,0},'R'),   {0,0,-15}));
    assert(eq(next_target({0,0,170},'L'), {0,0,-175}));      // turn_wraps
    assert(eq(next_target({0,0,-170},'R'),{0,0,175}));       // turn_wraps
    assert(eq(next_target({5,6,7},'X'),   {5,6,7}));         // unknown command: unchanged
}
