=========================================================
Some benchmark notes:
//Some stats: Without Depth Buffer in Razer Laptop.
//10000 triangles. FPS=267.06
// 1000 triangles. FPS=2034.21
//  500 triangles. FPS=3188.81
//  100 triangles. FPS=3822.71
//   10 triangles. FPS=3918.38 .. 4187.11
// With Depth Buffer
//10000 triangles. FPS=809.26
// 1000 triangles. FPS=3162.90
//  500 triangles. FPS=3622.99
//  100 triangles. FPS=3512.64
//   10 triangles. FPS=3662.17

//Stats after enabling worldMatrix before refactoring as ECS in Razer Laptop.
/* 1000 Triangles. Release mode
Frequency:2.15625e+06Period : 4.63768e-07, start : 795767951572, end : 795802159551
    tick count : 34207979, secs : 15.8646, ms : 15864.6, us : 1.58646e+07, ns : 1.58646e+10
    ticks / loop : 1111.08, secs / loop : 0.000515284, ms / loop : 0.515284, us / loop : 515.284, ns / loop : 515284
    frameCnt = 30788 FPS = 1940.68
    */
/* 1000 Triangles. Debug mode
Frequency:2.15625e+06Period:4.63768e-07, start:796160997255, end:796193166532
tick count:32169277, secs:14.9191, ms:14919.1, us:1.49191e+07, ns:1.49191e+10
ticks/loop:2169.93, secs/loop:0.00100635, ms/loop:1.00635, us/loop:1006.35, ns/loop:1.00635e+06
frameCnt=14825 FPS=993.69
    */
=========================================================

 Notes on std::map vs std::unordered_map performace
 See: YouTube video: C++Now 2018: You cando better than std::unordered_map by Malte Skarupke.
 Published by BoostCon channel.
 If you expect less than 400 keys use std::map
 if you expect between 400 to 100000 use std::unordered::map