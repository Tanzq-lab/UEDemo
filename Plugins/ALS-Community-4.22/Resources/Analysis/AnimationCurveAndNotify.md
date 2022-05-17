## 每个曲线对应的意思

|        曲线名称         |                             作用                             |
| :---------------------: | :----------------------------------------------------------: |
|     $BasePose\_CLF$     |                        处于蹲伏状态。                        |
|      $BasePose\_N$      |                         处于站立状态                         |
|   $Enable\_FootIK\_L$   |                  当值为1的时候启用左脚`IK`                   |
|   $Enable\_FootIK\_R$   |                  当值为1的时候启用右脚`IK`                   |
|   $Enable\_HandIK\_L$   |                  当值为1的时候启用左手`IK`                   |
|   $Enable\_HandIK\_R$   |                  当值为1的时候启用右手`IK`                   |
|  $Enable\_Transition$   | 当值小于零的时候不能使用过渡动画，其他情况可以使用。这里值虽然是负数，但是他的值会和其他曲线进行叠加，从而导致他的值是正数。 |
|    $Feet\_Crossing$     |                当值为1的时候处于脚步交叉状态                 |
|    $Feet\_Position$     |             小于零是左脚停止，右脚在运动，反之。             |
|      $FootLock\_L$      |                   当该值为1时禁止锁住左脚                    |
|      $FootLock\_R$      |                   当该值为1时禁止锁住右脚                    |
|   $Layering\_Arm\_L$    |                        左手臂混合比例                        |
| $Layering\_Arm\_L\_Add$ |                        左手臂叠加比例                        |
| $Layering\_Arm\_L\_LS$  |                      左手臂局部混合比例                      |
|   $Layering\_Arm\_R$    |                        右手臂混合比例                        |
| $Layering\_Arm\_R\_Add$ |                        右手臂叠加比例                        |
| $Layering\_Arm\_R\_LS$  |                      右手臂局部混合比例                      |
|   $Layering\_Hand\_L$   |                         左手混合比例                         |
|   $Layering\_Hand\_R$   |                         右手混合比例                         |
|    $Layering\_Head$     |                         头部混合比例                         |
|  $Layering\_Head\_Add$  |                       头部叠加混合比例                       |
|    $Layering\_Legs$     |               单独对腿部的层级控制，值为 -0.5                |
|   $Layering\_Pelvis$    |                         骨盆混合比例                         |
|    $Layering\_Spine$    |                         脊柱混合比例                         |
| $Layering\_Spine\_Add$  |                       脊柱叠加混合比例                       |
|    $Mask\_AimOffset$    |              当值为1的时候不启用瞄准偏差的混合               |
|  $Mask\_FootstepSound$  |                 当该值为1时禁止播放脚步声音                  |
| $Mask\_LandPrediction$  | 当该值为1的时候不能进行落地检测，只有当动画播放到快要落地的时候让该值逐渐变为0 |
|      $Mask\_Lean$       |                     当该值为1时禁止倾斜                      |
|     $Mask\_Sprint$      |                     当该值为1时禁止冲刺                      |
|        $W\_Gait$        |                 1：行走， 2：跑步， 3：冲刺                  |
|    $RotationAmount$     |                  -引擎根据数据生成的曲线值-                  |
|     $Weight\_InAir$     |                                                              |
|       $YawOffset$       |                                                              |
|         $Dummy$         |                                                              |
| $Enable\_SpineRotation$ |                                                              |
| $HipOrientation\_Bias$  |                                                              |