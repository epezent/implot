#pragma once

// convenience macros for using named colours in RGBA processes in imgui
// format (where 255 or 0.00f is the alpha channel) is:-
//ImU32 packedbyte = IM_COLRGBA(RGB_lavenderblush, 255);
//ImVec4 floatColor = ImVec4(CF_firebrick4, 0.00f);



#define RGB2FLOAT(r,g,b) (float)r/255.0f, (float)g/255.0f, (float)b/255.0f
#define IM_COLRGBA(RGB,A) (ImU32)(RGB | A << 24)

#define RGB_ghostwhite    248  |  248 <<8  |  255 <<16
#define RGB_whitesmoke    245  |  245 <<8  |  245 <<16
#define RGB_gainsboro    220  |  220 <<8  |  220 <<16
#define RGB_floralwhite    255  |  250 <<8  |  240 <<16
#define RGB_oldlace    253  |  245 <<8  |  230 <<16
#define RGB_linen    250  |  240 <<8  |  230 <<16
#define RGB_antiquewhite    250  |  235 <<8  |  215 <<16
#define RGB_papayawhip    255  |  239 <<8  |  213 <<16
#define RGB_blanchedalmond    255  |  235 <<8  |  205 <<16
#define RGB_mintcream    245  |  255 <<8  |  250 <<16
#define RGB_aliceblue    240  |  248 <<8  |  255 <<16
#define RGB_lavender    230  |  230 <<8  |  250 <<16
#define RGB_lavenderblush    255  |  240 <<8  |  245 <<16
#define RGB_lavenderblush1    255  |  240 <<8  |  245 <<16
#define RGB_snow    255  |  250 <<8  |  250 <<16
#define RGB_snow1    255  |  250 <<8  |  250 <<16
#define RGB_snow2    238  |  233 <<8  |  233 <<16
#define RGB_snow3    205  |  201 <<8  |  201 <<16
#define RGB_snow4    139  |  137 <<8  |  137 <<16
#define RGB_seashell    255  |  245 <<8  |  238 <<16
#define RGB_seashell1    255  |  245 <<8  |  238 <<16
#define RGB_seashell2    238  |  229 <<8  |  222 <<16
#define RGB_seashell3    205  |  197 <<8  |  191 <<16
#define RGB_seashell4    139  |  134 <<8  |  130 <<16
#define RGB_antiquewhite1    255  |  239 <<8  |  219 <<16
#define RGB_antiquewhite2    238  |  223 <<8  |  204 <<16
#define RGB_antiquewhite3    205  |  192 <<8  |  176 <<16
#define RGB_antiquewhite4    139  |  131 <<8  |  120 <<16
#define RGB_bisque    255  |  228 <<8  |  196 <<16
#define RGB_bisque1    255  |  228 <<8  |  196 <<16
#define RGB_bisque2    238  |  213 <<8  |  183 <<16
#define RGB_bisque3    205  |  183 <<8  |  158 <<16
#define RGB_bisque4    139  |  125 <<8  |  107 <<16
#define RGB_peachpuff    255  |  218 <<8  |  185 <<16
#define RGB_peachpuff1    255  |  218 <<8  |  185 <<16
#define RGB_peachpuff2    238  |  203 <<8  |  173 <<16
#define RGB_peachpuff3    205  |  175 <<8  |  149 <<16
#define RGB_peachpuff4    139  |  119 <<8  |  101 <<16
#define RGB_navajowhite    255  |  222 <<8  |  173 <<16
#define RGB_navajowhite1    255  |  222 <<8  |  173 <<16
#define RGB_navajowhite2    238  |  207 <<8  |  161 <<16
#define RGB_navajowhite3    205  |  179 <<8  |  139 <<16
#define RGB_navajowhite4    139  |  121 <<8  |  94 <<16
#define RGB_moccasin    255  |  228 <<8  |  181 <<16
#define RGB_lemonchiffon    255  |  250 <<8  |  205 <<16
#define RGB_lemonchiffon1    255  |  250 <<8  |  205 <<16
#define RGB_lemonchiffon2    238  |  233 <<8  |  191 <<16
#define RGB_lemonchiffon3    205  |  201 <<8  |  165 <<16
#define RGB_lemonchiffon4    139  |  137 <<8  |  112 <<16
#define RGB_beige    245  |  245 <<8  |  220 <<16
#define RGB_cornsilk    255  |  248 <<8  |  220 <<16
#define RGB_cornsilk1    255  |  248 <<8  |  220 <<16
#define RGB_cornsilk2    238  |  232 <<8  |  205 <<16
#define RGB_cornsilk3    205  |  200 <<8  |  177 <<16
#define RGB_cornsilk4    139  |  136 <<8  |  120 <<16
#define RGB_ivory    255  |  255 <<8  |  240 <<16
#define RGB_ivory1    255  |  255 <<8  |  240 <<16
#define RGB_ivory2    238  |  238 <<8  |  224 <<16
#define RGB_ivory3    205  |  205 <<8  |  193 <<16
#define RGB_ivory4    139  |  139 <<8  |  131 <<16
#define RGB_honeydew    240  |  255 <<8  |  240 <<16
#define RGB_honeydew1    240  |  255 <<8  |  240 <<16
#define RGB_honeydew2    224  |  238 <<8  |  224 <<16
#define RGB_honeydew3    193  |  205 <<8  |  193 <<16
#define RGB_honeydew4    131  |  139 <<8  |  131 <<16
#define RGB_lavenderblush2    238  |  224 <<8  |  229 <<16
#define RGB_lavenderblush3    205  |  193 <<8  |  197 <<16
#define RGB_lavenderblush4    139  |  131 <<8  |  134 <<16
#define RGB_mistyrose    255  |  228 <<8  |  225 <<16
#define RGB_mistyrose1    255  |  228 <<8  |  225 <<16
#define RGB_mistyrose2    238  |  213 <<8  |  210 <<16
#define RGB_mistyrose3    205  |  183 <<8  |  181 <<16
#define RGB_mistyrose4    139  |  125 <<8  |  123 <<16
#define RGB_azure    240  |  255 <<8  |  255 <<16
#define RGB_azure1    240  |  255 <<8  |  255 <<16
#define RGB_azure2    224  |  238 <<8  |  238 <<16
#define RGB_azure3    193  |  205 <<8  |  205 <<16
#define RGB_azure4    131  |  139 <<8  |  139 <<16
#define RGB_lightslateblue    132  |  112 <<8  |  255 <<16
#define RGB_mediumslateblue    123  |  104 <<8  |  238 <<16
#define RGB_darkslateblue    72  |  61 <<8  |  139 <<16
#define RGB_slateblue    106  |  90 <<8  |  205 <<16
#define RGB_slateblue1    131  |  111 <<8  |  255 <<16
#define RGB_slateblue2    122  |  103 <<8  |  238 <<16
#define RGB_slateblue3    105  |  89 <<8  |  205 <<16
#define RGB_slateblue4    71  |  60 <<8  |  139 <<16
#define RGB_royalblue    65  |  105 <<8  |  225 <<16
#define RGB_royalblue1    72  |  118 <<8  |  255 <<16
#define RGB_royalblue2    67  |  110 <<8  |  238 <<16
#define RGB_royalblue3    58  |  95 <<8  |  205 <<16
#define RGB_royalblue4    39  |  64 <<8  |  139 <<16
#define RGB_blue    0  |  0 <<8  |  255 <<16
#define RGB_blue1    0  |  0 <<8  |  255 <<16
#define RGB_blue2    0  |  0 <<8  |  238 <<16
#define RGB_blue3    0  |  0 <<8  |  205 <<16
#define RGB_blue4    0  |  0 <<8  |  139 <<16
#define RGB_navy    0  |  0 <<8  |  128 <<16
#define RGB_navyblue    0  |  0 <<8  |  128 <<16
#define RGB_darkblue    0  |  0 <<8  |  139 <<16
#define RGB_midnightblue    25  |  25 <<8  |  112 <<16
#define RGB_dodgerblue    30  |  144 <<8  |  255 <<16
#define RGB_dodgerblue1    30  |  144 <<8  |  255 <<16
#define RGB_dodgerblue2    28  |  134 <<8  |  238 <<16
#define RGB_dodgerblue3    24  |  116 <<8  |  205 <<16
#define RGB_dodgerblue4    16  |  78 <<8  |  139 <<16
#define RGB_steelblue    70  |  130 <<8  |  180 <<16
#define RGB_steelblue1    99  |  184 <<8  |  255 <<16
#define RGB_steelblue2    92  |  172 <<8  |  238 <<16
#define RGB_steelblue3    79  |  148 <<8  |  205 <<16
#define RGB_steelblue4    54  |  100 <<8  |  139 <<16
#define RGB_deepskyblue    0  |  191 <<8  |  255 <<16
#define RGB_deepskyblue1    0  |  191 <<8  |  255 <<16
#define RGB_deepskyblue2    0  |  178 <<8  |  238 <<16
#define RGB_deepskyblue3    0  |  154 <<8  |  205 <<16
#define RGB_deepskyblue4    0  |  104 <<8  |  139 <<16
#define RGB_skyblue    135  |  206 <<8  |  235 <<16
#define RGB_skyblue1    135  |  206 <<8  |  255 <<16
#define RGB_skyblue2    126  |  192 <<8  |  238 <<16
#define RGB_skyblue3    108  |  166 <<8  |  205 <<16
#define RGB_skyblue4    74  |  112 <<8  |  139 <<16
#define RGB_cornflowerblue    100  |  149 <<8  |  237 <<16
#define RGB_mediumblue    0  |  0 <<8  |  205 <<16
#define RGB_lightskyblue    135  |  206 <<8  |  250 <<16
#define RGB_lightskyblue1    176  |  226 <<8  |  255 <<16
#define RGB_lightskyblue2    164  |  211 <<8  |  238 <<16
#define RGB_lightskyblue3    141  |  182 <<8  |  205 <<16
#define RGB_lightskyblue4    96  |  123 <<8  |  139 <<16
#define RGB_lightslategray    119  |  136 <<8  |  153 <<16
#define RGB_lightslategrey    119  |  136 <<8  |  153 <<16
#define RGB_slategray    112  |  128 <<8  |  144 <<16
#define RGB_slategrey    112  |  128 <<8  |  144 <<16
#define RGB_slategray1    198  |  226 <<8  |  255 <<16
#define RGB_slategray2    185  |  211 <<8  |  238 <<16
#define RGB_slategray3    159  |  182 <<8  |  205 <<16
#define RGB_slategray4    108  |  123 <<8  |  139 <<16
#define RGB_lightsteelblue1    202  |  225 <<8  |  255 <<16
#define RGB_lightsteelblue2    188  |  210 <<8  |  238 <<16
#define RGB_lightsteelblue3    162  |  181 <<8  |  205 <<16
#define RGB_lightsteelblue4    110  |  123 <<8  |  139 <<16
#define RGB_lightblue1    191  |  239 <<8  |  255 <<16
#define RGB_lightblue2    178  |  223 <<8  |  238 <<16
#define RGB_lightblue3    154  |  192 <<8  |  205 <<16
#define RGB_lightblue4    104  |  131 <<8  |  139 <<16
#define RGB_lightcyan    224  |  255 <<8  |  255 <<16
#define RGB_lightcyan1    224  |  255 <<8  |  255 <<16
#define RGB_lightcyan2    209  |  238 <<8  |  238 <<16
#define RGB_lightcyan3    180  |  205 <<8  |  205 <<16
#define RGB_lightcyan4    122  |  139 <<8  |  139 <<16
#define RGB_paleturquoise1    187  |  255 <<8  |  255 <<16
#define RGB_paleturquoise2    174  |  238 <<8  |  238 <<16
#define RGB_paleturquoise3    150  |  205 <<8  |  205 <<16
#define RGB_paleturquoise4    102  |  139 <<8  |  139 <<16
#define RGB_cadetblue1    152  |  245 <<8  |  255 <<16
#define RGB_cadetblue2    142  |  229 <<8  |  238 <<16
#define RGB_cadetblue3    122  |  197 <<8  |  205 <<16
#define RGB_cadetblue4    83  |  134 <<8  |  139 <<16
#define RGB_turquoise1    0  |  245 <<8  |  255 <<16
#define RGB_turquoise2    0  |  229 <<8  |  238 <<16
#define RGB_turquoise3    0  |  197 <<8  |  205 <<16
#define RGB_turquoise4    0  |  134 <<8  |  139 <<16
#define RGB_lightsteelblue    176  |  196 <<8  |  222 <<16
#define RGB_lightblue    173  |  216 <<8  |  230 <<16
#define RGB_powderblue    176  |  224 <<8  |  230 <<16
#define RGB_paleturquoise    175  |  238 <<8  |  238 <<16
#define RGB_darkturquoise    0  |  206 <<8  |  209 <<16
#define RGB_mediumturquoise    72  |  209 <<8  |  204 <<16
#define RGB_turquoise    64  |  224 <<8  |  208 <<16
#define RGB_cadetblue    95  |  158 <<8  |  160 <<16
#define RGB_lightgreen    144  |  238 <<8  |  144 <<16
#define RGB_darkgreen    0  |  100 <<8  |  0 <<16
#define RGB_darkolivegreen    85  |  107 <<8  |  47 <<16
#define RGB_palegreen    152  |  251 <<8  |  152 <<16
#define RGB_lawngreen    124  |  252 <<8  |  0 <<16
#define RGB_greenyellow    173  |  255 <<8  |  47 <<16
#define RGB_limegreen    50  |  205 <<8  |  50 <<16
#define RGB_yellowgreen    154  |  205 <<8  |  50 <<16
#define RGB_forestgreen    34  |  139 <<8  |  34 <<16
#define RGB_cyan    0  |  255 <<8  |  255 <<16
#define RGB_cyan1    0  |  255 <<8  |  255 <<16
#define RGB_cyan2    0  |  238 <<8  |  238 <<16
#define RGB_cyan3    0  |  205 <<8  |  205 <<16
#define RGB_cyan4    0  |  139 <<8  |  139 <<16
#define RGB_darkcyan    0  |  139 <<8  |  139 <<16
#define RGB_darkslategray1    151  |  255 <<8  |  255 <<16
#define RGB_darkslategray2    141  |  238 <<8  |  238 <<16
#define RGB_darkslategray3    121  |  205 <<8  |  205 <<16
#define RGB_darkslategray4    82  |  139 <<8  |  139 <<16
#define RGB_darkslategray    47  |  79 <<8  |  79 <<16
#define RGB_darkslategrey    47  |  79 <<8  |  79 <<16
#define RGB_aquamarine    127  |  255 <<8  |  212 <<16
#define RGB_aquamarine1    127  |  255 <<8  |  212 <<16
#define RGB_aquamarine2    118  |  238 <<8  |  198 <<16
#define RGB_aquamarine3    102  |  205 <<8  |  170 <<16
#define RGB_aquamarine4    69  |  139 <<8  |  116 <<16
#define RGB_mediumaquamarine    102  |  205 <<8  |  170 <<16
#define RGB_mediumseagreen    60  |  179 <<8  |  113 <<16
#define RGB_lightseagreen    32  |  178 <<8  |  170 <<16
#define RGB_seagreen    46  |  139 <<8  |  87 <<16
#define RGB_seagreen1    84  |  255 <<8  |  159 <<16
#define RGB_seagreen2    78  |  238 <<8  |  148 <<16
#define RGB_seagreen3    67  |  205 <<8  |  128 <<16
#define RGB_seagreen4    46  |  139 <<8  |  87 <<16
#define RGB_darkseagreen    143  |  188 <<8  |  143 <<16
#define RGB_darkseagreen1    193  |  255 <<8  |  193 <<16
#define RGB_darkseagreen2    180  |  238 <<8  |  180 <<16
#define RGB_darkseagreen3    155  |  205 <<8  |  155 <<16
#define RGB_darkseagreen4    105  |  139 <<8  |  105 <<16
#define RGB_palegreen1    154  |  255 <<8  |  154 <<16
#define RGB_palegreen2    144  |  238 <<8  |  144 <<16
#define RGB_palegreen3    124  |  205 <<8  |  124 <<16
#define RGB_palegreen4    84  |  139 <<8  |  84 <<16
#define RGB_mediumspringgreen    0  |  250 <<8  |  154 <<16
#define RGB_springgreen    0  |  255 <<8  |  127 <<16
#define RGB_springgreen1    0  |  255 <<8  |  127 <<16
#define RGB_springgreen2    0  |  238 <<8  |  118 <<16
#define RGB_springgreen3    0  |  205 <<8  |  102 <<16
#define RGB_springgreen4    0  |  139 <<8  |  69 <<16
#define RGB_green    0  |  255 <<8  |  0 <<16
#define RGB_green1    0  |  255 <<8  |  0 <<16
#define RGB_green2    0  |  238 <<8  |  0 <<16
#define RGB_green3    0  |  205 <<8  |  0 <<16
#define RGB_green4    0  |  139 <<8  |  0 <<16
#define RGB_chartreuse    127  |  255 <<8  |  0 <<16
#define RGB_chartreuse1    127  |  255 <<8  |  0 <<16
#define RGB_chartreuse2    118  |  238 <<8  |  0 <<16
#define RGB_chartreuse3    102  |  205 <<8  |  0 <<16
#define RGB_chartreuse4    69  |  139 <<8  |  0 <<16
#define RGB_olivedrab    107  |  142 <<8  |  35 <<16
#define RGB_olivedrab1    192  |  255 <<8  |  62 <<16
#define RGB_olivedrab2    179  |  238 <<8  |  58 <<16
#define RGB_olivedrab3    154  |  205 <<8  |  50 <<16
#define RGB_olivedrab4    105  |  139 <<8  |  34 <<16
#define RGB_darkolivegreen1    202  |  255 <<8  |  112 <<16
#define RGB_darkolivegreen2    188  |  238 <<8  |  104 <<16
#define RGB_darkolivegreen3    162  |  205 <<8  |  90 <<16
#define RGB_darkolivegreen4    110  |  139 <<8  |  61 <<16
#define RGB_darkkhaki    189  |  183 <<8  |  107 <<16
#define RGB_khaki    240  |  230 <<8  |  140 <<16
#define RGB_khaki1    255  |  246 <<8  |  143 <<16
#define RGB_khaki2    238  |  230 <<8  |  133 <<16
#define RGB_khaki3    205  |  198 <<8  |  115 <<16
#define RGB_khaki4    139  |  134 <<8  |  78 <<16
#define RGB_lightyellow    255  |  255 <<8  |  224 <<16
#define RGB_lightyellow1    255  |  255 <<8  |  224 <<16
#define RGB_lightyellow2    238  |  238 <<8  |  209 <<16
#define RGB_lightyellow3    205  |  205 <<8  |  180 <<16
#define RGB_lightyellow4    139  |  139 <<8  |  122 <<16
#define RGB_yellow    255  |  255 <<8  |  0 <<16
#define RGB_yellow1    255  |  255 <<8  |  0 <<16
#define RGB_yellow2    238  |  238 <<8  |  0 <<16
#define RGB_yellow3    205  |  205 <<8  |  0 <<16
#define RGB_yellow4    139  |  139 <<8  |  0 <<16
#define RGB_gold    255  |  215 <<8  |  0 <<16
#define RGB_gold1    255  |  215 <<8  |  0 <<16
#define RGB_gold2    238  |  201 <<8  |  0 <<16
#define RGB_gold3    205  |  173 <<8  |  0 <<16
#define RGB_gold4    139  |  117 <<8  |  0 <<16
#define RGB_goldenrod    218  |  165 <<8  |  32 <<16
#define RGB_goldenrod1    255  |  193 <<8  |  37 <<16
#define RGB_goldenrod2    238  |  180 <<8  |  34 <<16
#define RGB_goldenrod3    205  |  155 <<8  |  29 <<16
#define RGB_goldenrod4    139  |  105 <<8  |  20 <<16
#define RGB_lightgoldenrodyellow    250  |  250 <<8  |  210 <<16
#define RGB_lightgoldenrod    238  |  221 <<8  |  130 <<16
#define RGB_lightgoldenrod1    255  |  236 <<8  |  139 <<16
#define RGB_lightgoldenrod2    238  |  220 <<8  |  130 <<16
#define RGB_lightgoldenrod3    205  |  190 <<8  |  112 <<16
#define RGB_lightgoldenrod4    139  |  129 <<8  |  76 <<16
#define RGB_palegoldenrod    238  |  232 <<8  |  170 <<16
#define RGB_darkgoldenrod    184  |  134 <<8  |  11 <<16
#define RGB_darkgoldenrod1    255  |  185 <<8  |  15 <<16
#define RGB_darkgoldenrod2    238  |  173 <<8  |  14 <<16
#define RGB_darkgoldenrod3    205  |  149 <<8  |  12 <<16
#define RGB_darkgoldenrod4    139  |  101 <<8  |  8 <<16
#define RGB_rosybrown    188  |  143 <<8  |  143 <<16
#define RGB_rosybrown1    255  |  193 <<8  |  193 <<16
#define RGB_rosybrown2    238  |  180 <<8  |  180 <<16
#define RGB_rosybrown3    205  |  155 <<8  |  155 <<16
#define RGB_rosybrown4    139  |  105 <<8  |  105 <<16
#define RGB_indianred    205  |  92 <<8  |  92 <<16
#define RGB_indianred1    255  |  106 <<8  |  106 <<16
#define RGB_indianred2    238  |  99 <<8  |  99 <<16
#define RGB_indianred3    205  |  85 <<8  |  85 <<16
#define RGB_indianred4    139  |  58 <<8  |  58 <<16
#define RGB_saddlebrown    139  |  69 <<8  |  19 <<16
#define RGB_sandybrown    244  |  164 <<8  |  96 <<16
#define RGB_peru    205  |  133 <<8  |  63 <<16
#define RGB_sienna    160  |  82 <<8  |  45 <<16
#define RGB_sienna1    255  |  130 <<8  |  71 <<16
#define RGB_sienna2    238  |  121 <<8  |  66 <<16
#define RGB_sienna3    205  |  104 <<8  |  57 <<16
#define RGB_sienna4    139  |  71 <<8  |  38 <<16
#define RGB_burlywood    222  |  184 <<8  |  135 <<16
#define RGB_burlywood1    255  |  211 <<8  |  155 <<16
#define RGB_burlywood2    238  |  197 <<8  |  145 <<16
#define RGB_burlywood3    205  |  170 <<8  |  125 <<16
#define RGB_burlywood4    139  |  115 <<8  |  85 <<16
#define RGB_wheat    245  |  222 <<8  |  179 <<16
#define RGB_wheat1    255  |  231 <<8  |  186 <<16
#define RGB_wheat2    238  |  216 <<8  |  174 <<16
#define RGB_wheat3    205  |  186 <<8  |  150 <<16
#define RGB_wheat4    139  |  126 <<8  |  102 <<16
#define RGB_tan    210  |  180 <<8  |  140 <<16
#define RGB_tan1    255  |  165 <<8  |  79 <<16
#define RGB_tan2    238  |  154 <<8  |  73 <<16
#define RGB_tan3    205  |  133 <<8  |  63 <<16
#define RGB_tan4    139  |  90 <<8  |  43 <<16
#define RGB_chocolate    210  |  105 <<8  |  30 <<16
#define RGB_chocolate1    255  |  127 <<8  |  36 <<16
#define RGB_chocolate2    238  |  118 <<8  |  33 <<16
#define RGB_chocolate3    205  |  102 <<8  |  29 <<16
#define RGB_chocolate4    139  |  69 <<8  |  19 <<16
#define RGB_firebrick    178  |  34 <<8  |  34 <<16
#define RGB_firebrick1    255  |  48 <<8  |  48 <<16
#define RGB_firebrick2    238  |  44 <<8  |  44 <<16
#define RGB_firebrick3    205  |  38 <<8  |  38 <<16
#define RGB_firebrick4    139  |  26 <<8  |  26 <<16
#define RGB_brown    165  |  42 <<8  |  42 <<16
#define RGB_brown1    255  |  64 <<8  |  64 <<16
#define RGB_brown2    238  |  59 <<8  |  59 <<16
#define RGB_brown3    205  |  51 <<8  |  51 <<16
#define RGB_brown4    139  |  35 <<8  |  35 <<16
#define RGB_salmon    250  |  128 <<8  |  114 <<16
#define RGB_salmon1    255  |  140 <<8  |  105 <<16
#define RGB_salmon2    238  |  130 <<8  |  98 <<16
#define RGB_salmon3    205  |  112 <<8  |  84 <<16
#define RGB_salmon4    139  |  76 <<8  |  57 <<16
#define RGB_darksalmon    233  |  150 <<8  |  122 <<16
#define RGB_lightsalmon    255  |  160 <<8  |  122 <<16
#define RGB_lightsalmon1    255  |  160 <<8  |  122 <<16
#define RGB_lightsalmon2    238  |  149 <<8  |  114 <<16
#define RGB_lightsalmon3    205  |  129 <<8  |  98 <<16
#define RGB_lightsalmon4    139  |  87 <<8  |  66 <<16
#define RGB_orange    255  |  165 <<8  |  0 <<16
#define RGB_orange1    255  |  165 <<8  |  0 <<16
#define RGB_orange2    238  |  154 <<8  |  0 <<16
#define RGB_orange3    205  |  133 <<8  |  0 <<16
#define RGB_orange4    139  |  90 <<8  |  0 <<16
#define RGB_darkorange    255  |  140 <<8  |  0 <<16
#define RGB_darkorange1    255  |  127 <<8  |  0 <<16
#define RGB_darkorange2    238  |  118 <<8  |  0 <<16
#define RGB_darkorange3    205  |  102 <<8  |  0 <<16
#define RGB_darkorange4    139  |  69 <<8  |  0 <<16
#define RGB_lightcoral    240  |  128 <<8  |  128 <<16
#define RGB_coral    255  |  127 <<8  |  80 <<16
#define RGB_coral1    255  |  114 <<8  |  86 <<16
#define RGB_coral2    238  |  106 <<8  |  80 <<16
#define RGB_coral3    205  |  91 <<8  |  69 <<16
#define RGB_coral4    139  |  62 <<8  |  47 <<16
#define RGB_tomato    255  |  99 <<8  |  71 <<16
#define RGB_tomato1    255  |  99 <<8  |  71 <<16
#define RGB_tomato2    238  |  92 <<8  |  66 <<16
#define RGB_tomato3    205  |  79 <<8  |  57 <<16
#define RGB_tomato4    139  |  54 <<8  |  38 <<16
#define RGB_orangered    255  |  69 <<8  |  0 <<16
#define RGB_orangered1    255  |  69 <<8  |  0 <<16
#define RGB_orangered2    238  |  64 <<8  |  0 <<16
#define RGB_orangered3    205  |  55 <<8  |  0 <<16
#define RGB_orangered4    139  |  37 <<8  |  0 <<16
#define RGB_red    255  |  0 <<8  |  0 <<16
#define RGB_red1    255  |  0 <<8  |  0 <<16
#define RGB_red2    238  |  0 <<8  |  0 <<16
#define RGB_red3    205  |  0 <<8  |  0 <<16
#define RGB_red4    139  |  0 <<8  |  0 <<16
#define RGB_darkred    139  |  0 <<8  |  0 <<16
#define RGB_deeppink    255  |  20 <<8  |  147 <<16
#define RGB_deeppink1    255  |  20 <<8  |  147 <<16
#define RGB_deeppink2    238  |  18 <<8  |  137 <<16
#define RGB_deeppink3    205  |  16 <<8  |  118 <<16
#define RGB_deeppink4    139  |  10 <<8  |  80 <<16
#define RGB_hotpink    255  |  105 <<8  |  180 <<16
#define RGB_hotpink1    255  |  110 <<8  |  180 <<16
#define RGB_hotpink2    238  |  106 <<8  |  167 <<16
#define RGB_hotpink3    205  |  96 <<8  |  144 <<16
#define RGB_hotpink4    139  |  58 <<8  |  98 <<16
#define RGB_pink    255  |  192 <<8  |  203 <<16
#define RGB_pink1    255  |  181 <<8  |  197 <<16
#define RGB_pink2    238  |  169 <<8  |  184 <<16
#define RGB_pink3    205  |  145 <<8  |  158 <<16
#define RGB_pink4    139  |  99 <<8  |  108 <<16
#define RGB_lightpink    255  |  182 <<8  |  193 <<16
#define RGB_lightpink1    255  |  174 <<8  |  185 <<16
#define RGB_lightpink2    238  |  162 <<8  |  173 <<16
#define RGB_lightpink3    205  |  140 <<8  |  149 <<16
#define RGB_lightpink4    139  |  95 <<8  |  101 <<16
#define RGB_palevioletred    219  |  112 <<8  |  147 <<16
#define RGB_palevioletred1    255  |  130 <<8  |  171 <<16
#define RGB_palevioletred2    238  |  121 <<8  |  159 <<16
#define RGB_palevioletred3    205  |  104 <<8  |  137 <<16
#define RGB_palevioletred4    139  |  71 <<8  |  93 <<16
#define RGB_maroon    176  |  48 <<8  |  96 <<16
#define RGB_maroon1    255  |  52 <<8  |  179 <<16
#define RGB_maroon2    238  |  48 <<8  |  167 <<16
#define RGB_maroon3    205  |  41 <<8  |  144 <<16
#define RGB_maroon4    139  |  28 <<8  |  98 <<16
#define RGB_mediumvioletred    199  |  21 <<8  |  133 <<16
#define RGB_violet    238  |  130 <<8  |  238 <<16
#define RGB_violetred    208  |  32 <<8  |  144 <<16
#define RGB_violetred1    255  |  62 <<8  |  150 <<16
#define RGB_violetred2    238  |  58 <<8  |  140 <<16
#define RGB_violetred3    205  |  50 <<8  |  120 <<16
#define RGB_violetred4    139  |  34 <<8  |  82 <<16
#define RGB_magenta    255  |  0 <<8  |  255 <<16
#define RGB_magenta1    255  |  0 <<8  |  255 <<16
#define RGB_magenta2    238  |  0 <<8  |  238 <<16
#define RGB_magenta3    205  |  0 <<8  |  205 <<16
#define RGB_magenta4    139  |  0 <<8  |  139 <<16
#define RGB_darkmagenta    139  |  0 <<8  |  139 <<16
#define RGB_orchid    218  |  112 <<8  |  214 <<16
#define RGB_orchid1    255  |  131 <<8  |  250 <<16
#define RGB_orchid2    238  |  122 <<8  |  233 <<16
#define RGB_orchid3    205  |  105 <<8  |  201 <<16
#define RGB_orchid4    139  |  71 <<8  |  137 <<16
#define RGB_plum    221  |  160 <<8  |  221 <<16
#define RGB_plum1    255  |  187 <<8  |  255 <<16
#define RGB_plum2    238  |  174 <<8  |  238 <<16
#define RGB_plum3    205  |  150 <<8  |  205 <<16
#define RGB_plum4    139  |  102 <<8  |  139 <<16
#define RGB_mediumorchid    186  |  85 <<8  |  211 <<16
#define RGB_mediumorchid1    224  |  102 <<8  |  255 <<16
#define RGB_mediumorchid2    209  |  95 <<8  |  238 <<16
#define RGB_mediumorchid3    180  |  82 <<8  |  205 <<16
#define RGB_mediumorchid4    122  |  55 <<8  |  139 <<16
#define RGB_darkorchid    153  |  50 <<8  |  204 <<16
#define RGB_darkorchid1    191  |  62 <<8  |  255 <<16
#define RGB_darkorchid2    178  |  58 <<8  |  238 <<16
#define RGB_darkorchid3    154  |  50 <<8  |  205 <<16
#define RGB_darkorchid4    104  |  34 <<8  |  139 <<16
#define RGB_purple    160  |  32 <<8  |  240 <<16
#define RGB_purple1    155  |  48 <<8  |  255 <<16
#define RGB_purple2    145  |  44 <<8  |  238 <<16
#define RGB_purple3    125  |  38 <<8  |  205 <<16
#define RGB_purple4    85  |  26 <<8  |  139 <<16
#define RGB_mediumpurple1    171  |  130 <<8  |  255 <<16
#define RGB_mediumpurple2    159  |  121 <<8  |  238 <<16
#define RGB_mediumpurple3    137  |  104 <<8  |  205 <<16
#define RGB_mediumpurple4    93  |  71 <<8  |  139 <<16
#define RGB_thistle    216  |  191 <<8  |  216 <<16
#define RGB_thistle1    255  |  225 <<8  |  255 <<16
#define RGB_thistle2    238  |  210 <<8  |  238 <<16
#define RGB_thistle3    205  |  181 <<8  |  205 <<16
#define RGB_thistle4    139  |  123 <<8  |  139 <<16
#define RGB_darkviolet    148  |  0 <<8  |  211 <<16
#define RGB_blueviolet    138  |  43 <<8  |  226 <<16
#define RGB_mediumpurple    147  |  112 <<8  |  219 <<16
#define RGB_black    0  |  0 <<8  |  0 <<16
#define RGB_gray0    0  |  0 <<8  |  0 <<16
#define RGB_gray1    3  |  3 <<8  |  3 <<16
#define RGB_gray2    5  |  5 <<8  |  5 <<16
#define RGB_gray3    8  |  8 <<8  |  8 <<16
#define RGB_gray4    10  |  10 <<8  |  10 <<16
#define RGB_gray5    13  |  13 <<8  |  13 <<16
#define RGB_gray6    15  |  15 <<8  |  15 <<16
#define RGB_gray7    18  |  18 <<8  |  18 <<16
#define RGB_gray8    20  |  20 <<8  |  20 <<16
#define RGB_gray9    23  |  23 <<8  |  23 <<16
#define RGB_gray10    26  |  26 <<8  |  26 <<16
#define RGB_gray11    28  |  28 <<8  |  28 <<16
#define RGB_gray12    31  |  31 <<8  |  31 <<16
#define RGB_gray13    33  |  33 <<8  |  33 <<16
#define RGB_gray14    36  |  36 <<8  |  36 <<16
#define RGB_gray15    38  |  38 <<8  |  38 <<16
#define RGB_gray16    41  |  41 <<8  |  41 <<16
#define RGB_gray17    43  |  43 <<8  |  43 <<16
#define RGB_gray18    46  |  46 <<8  |  46 <<16
#define RGB_gray19    48  |  48 <<8  |  48 <<16
#define RGB_gray20    51  |  51 <<8  |  51 <<16
#define RGB_gray21    54  |  54 <<8  |  54 <<16
#define RGB_gray22    56  |  56 <<8  |  56 <<16
#define RGB_gray23    59  |  59 <<8  |  59 <<16
#define RGB_gray24    61  |  61 <<8  |  61 <<16
#define RGB_gray25    64  |  64 <<8  |  64 <<16
#define RGB_gray26    66  |  66 <<8  |  66 <<16
#define RGB_gray27    69  |  69 <<8  |  69 <<16
#define RGB_gray28    71  |  71 <<8  |  71 <<16
#define RGB_gray29    74  |  74 <<8  |  74 <<16
#define RGB_gray30    77  |  77 <<8  |  77 <<16
#define RGB_gray31    79  |  79 <<8  |  79 <<16
#define RGB_gray32    82  |  82 <<8  |  82 <<16
#define RGB_gray33    84  |  84 <<8  |  84 <<16
#define RGB_gray34    87  |  87 <<8  |  87 <<16
#define RGB_gray35    89  |  89 <<8  |  89 <<16
#define RGB_gray36    92  |  92 <<8  |  92 <<16
#define RGB_gray37    94  |  94 <<8  |  94 <<16
#define RGB_gray38    97  |  97 <<8  |  97 <<16
#define RGB_gray39    99  |  99 <<8  |  99 <<16
#define RGB_gray40    102  |  102 <<8  |  102 <<16
#define RGB_gray41    105  |  105 <<8  |  105 <<16
#define RGB_dimgray    105  |  105 <<8  |  105 <<16
#define RGB_gray42    107  |  107 <<8  |  107 <<16
#define RGB_gray43    110  |  110 <<8  |  110 <<16
#define RGB_gray44    112  |  112 <<8  |  112 <<16
#define RGB_gray45    115  |  115 <<8  |  115 <<16
#define RGB_gray46    117  |  117 <<8  |  117 <<16
#define RGB_gray47    120  |  120 <<8  |  120 <<16
#define RGB_gray48    122  |  122 <<8  |  122 <<16
#define RGB_gray49    125  |  125 <<8  |  125 <<16
#define RGB_gray50    127  |  127 <<8  |  127 <<16
#define RGB_gray51    130  |  130 <<8  |  130 <<16
#define RGB_gray52    133  |  133 <<8  |  133 <<16
#define RGB_gray53    135  |  135 <<8  |  135 <<16
#define RGB_gray54    138  |  138 <<8  |  138 <<16
#define RGB_gray55    140  |  140 <<8  |  140 <<16
#define RGB_gray56    143  |  143 <<8  |  143 <<16
#define RGB_gray57    145  |  145 <<8  |  145 <<16
#define RGB_gray58    148  |  148 <<8  |  148 <<16
#define RGB_gray59    150  |  150 <<8  |  150 <<16
#define RGB_gray60    153  |  153 <<8  |  153 <<16
#define RGB_gray61    156  |  156 <<8  |  156 <<16
#define RGB_gray62    158  |  158 <<8  |  158 <<16
#define RGB_gray63    161  |  161 <<8  |  161 <<16
#define RGB_gray64    163  |  163 <<8  |  163 <<16
#define RGB_gray65    166  |  166 <<8  |  166 <<16
#define RGB_darkgray    169  |  169 <<8  |  169 <<16
#define RGB_gray66    168  |  168 <<8  |  168 <<16
#define RGB_gray67    171  |  171 <<8  |  171 <<16
#define RGB_gray68    173  |  173 <<8  |  173 <<16
#define RGB_gray69    176  |  176 <<8  |  176 <<16
#define RGB_gray70    179  |  179 <<8  |  179 <<16
#define RGB_gray71    181  |  181 <<8  |  181 <<16
#define RGB_gray72    184  |  184 <<8  |  184 <<16
#define RGB_gray73    186  |  186 <<8  |  186 <<16
#define RGB_gray74    189  |  189 <<8  |  189 <<16
#define RGB_gray    190  |  190 <<8  |  190 <<16
#define RGB_gray75    191  |  191 <<8  |  191 <<16
#define RGB_gray76    194  |  194 <<8  |  194 <<16
#define RGB_gray77    196  |  196 <<8  |  196 <<16
#define RGB_gray78    199  |  199 <<8  |  199 <<16
#define RGB_gray79    201  |  201 <<8  |  201 <<16
#define RGB_gray80    204  |  204 <<8  |  204 <<16
#define RGB_gray81    207  |  207 <<8  |  207 <<16
#define RGB_gray82    209  |  209 <<8  |  209 <<16
#define RGB_lightgray    211  |  211 <<8  |  211 <<16
#define RGB_gray83    212  |  212 <<8  |  212 <<16
#define RGB_gray84    214  |  214 <<8  |  214 <<16
#define RGB_gray85    217  |  217 <<8  |  217 <<16
#define RGB_gray86    219  |  219 <<8  |  219 <<16
#define RGB_gray87    222  |  222 <<8  |  222 <<16
#define RGB_gray88    224  |  224 <<8  |  224 <<16
#define RGB_gray89    227  |  227 <<8  |  227 <<16
#define RGB_gray90    229  |  229 <<8  |  229 <<16
#define RGB_gray91    232  |  232 <<8  |  232 <<16
#define RGB_gray92    235  |  235 <<8  |  235 <<16
#define RGB_gray93    237  |  237 <<8  |  237 <<16
#define RGB_gray94    240  |  240 <<8  |  240 <<16
#define RGB_gray95    242  |  242 <<8  |  242 <<16
#define RGB_gray96    245  |  245 <<8  |  245 <<16
#define RGB_gray97    247  |  247 <<8  |  247 <<16
#define RGB_gray98    250  |  250 <<8  |  250 <<16
#define RGB_gray99    252  |  252 <<8  |  252 <<16
#define RGB_gray100    255  |  255 <<8  |  255 <<16
#define RGB_white    255  |  255 <<8  |  255 <<16
#define RGB_grey0    0  |  0 <<8  |  0 <<16
#define RGB_grey1    3  |  3 <<8  |  3 <<16
#define RGB_grey2    5  |  5 <<8  |  5 <<16
#define RGB_grey3    8  |  8 <<8  |  8 <<16
#define RGB_grey4    10  |  10 <<8  |  10 <<16
#define RGB_grey5    13  |  13 <<8  |  13 <<16
#define RGB_grey6    15  |  15 <<8  |  15 <<16
#define RGB_grey7    18  |  18 <<8  |  18 <<16
#define RGB_grey8    20  |  20 <<8  |  20 <<16
#define RGB_grey9    23  |  23 <<8  |  23 <<16
#define RGB_grey10    26  |  26 <<8  |  26 <<16
#define RGB_grey11    28  |  28 <<8  |  28 <<16
#define RGB_grey12    31  |  31 <<8  |  31 <<16
#define RGB_grey13    33  |  33 <<8  |  33 <<16
#define RGB_grey14    36  |  36 <<8  |  36 <<16
#define RGB_grey15    38  |  38 <<8  |  38 <<16
#define RGB_grey16    41  |  41 <<8  |  41 <<16
#define RGB_grey17    43  |  43 <<8  |  43 <<16
#define RGB_grey18    46  |  46 <<8  |  46 <<16
#define RGB_grey19    48  |  48 <<8  |  48 <<16
#define RGB_grey20    51  |  51 <<8  |  51 <<16
#define RGB_grey21    54  |  54 <<8  |  54 <<16
#define RGB_grey22    56  |  56 <<8  |  56 <<16
#define RGB_grey23    59  |  59 <<8  |  59 <<16
#define RGB_grey24    61  |  61 <<8  |  61 <<16
#define RGB_grey25    64  |  64 <<8  |  64 <<16
#define RGB_grey26    66  |  66 <<8  |  66 <<16
#define RGB_grey27    69  |  69 <<8  |  69 <<16
#define RGB_grey28    71  |  71 <<8  |  71 <<16
#define RGB_grey29    74  |  74 <<8  |  74 <<16
#define RGB_grey30    77  |  77 <<8  |  77 <<16
#define RGB_grey31    79  |  79 <<8  |  79 <<16
#define RGB_grey32    82  |  82 <<8  |  82 <<16
#define RGB_grey33    84  |  84 <<8  |  84 <<16
#define RGB_grey34    87  |  87 <<8  |  87 <<16
#define RGB_grey35    89  |  89 <<8  |  89 <<16
#define RGB_grey36    92  |  92 <<8  |  92 <<16
#define RGB_grey37    94  |  94 <<8  |  94 <<16
#define RGB_grey38    97  |  97 <<8  |  97 <<16
#define RGB_grey39    99  |  99 <<8  |  99 <<16
#define RGB_grey40    102  |  102 <<8  |  102 <<16
#define RGB_grey41    105  |  105 <<8  |  105 <<16
#define RGB_dimgrey    105  |  105 <<8  |  105 <<16
#define RGB_grey42    107  |  107 <<8  |  107 <<16
#define RGB_grey43    110  |  110 <<8  |  110 <<16
#define RGB_grey44    112  |  112 <<8  |  112 <<16
#define RGB_grey45    115  |  115 <<8  |  115 <<16
#define RGB_grey46    117  |  117 <<8  |  117 <<16
#define RGB_grey47    120  |  120 <<8  |  120 <<16
#define RGB_grey48    122  |  122 <<8  |  122 <<16
#define RGB_grey49    125  |  125 <<8  |  125 <<16
#define RGB_grey50    127  |  127 <<8  |  127 <<16
#define RGB_grey51    130  |  130 <<8  |  130 <<16
#define RGB_grey52    133  |  133 <<8  |  133 <<16
#define RGB_grey53    135  |  135 <<8  |  135 <<16
#define RGB_grey54    138  |  138 <<8  |  138 <<16
#define RGB_grey55    140  |  140 <<8  |  140 <<16
#define RGB_grey56    143  |  143 <<8  |  143 <<16
#define RGB_grey57    145  |  145 <<8  |  145 <<16
#define RGB_grey58    148  |  148 <<8  |  148 <<16
#define RGB_grey59    150  |  150 <<8  |  150 <<16
#define RGB_grey60    153  |  153 <<8  |  153 <<16
#define RGB_grey61    156  |  156 <<8  |  156 <<16
#define RGB_grey62    158  |  158 <<8  |  158 <<16
#define RGB_grey63    161  |  161 <<8  |  161 <<16
#define RGB_grey64    163  |  163 <<8  |  163 <<16
#define RGB_grey65    166  |  166 <<8  |  166 <<16
#define RGB_darkgrey    169  |  169 <<8  |  169 <<16
#define RGB_grey66    168  |  168 <<8  |  168 <<16
#define RGB_grey67    171  |  171 <<8  |  171 <<16
#define RGB_grey68    173  |  173 <<8  |  173 <<16
#define RGB_grey69    176  |  176 <<8  |  176 <<16
#define RGB_grey70    179  |  179 <<8  |  179 <<16
#define RGB_grey71    181  |  181 <<8  |  181 <<16
#define RGB_grey72    184  |  184 <<8  |  184 <<16
#define RGB_grey73    186  |  186 <<8  |  186 <<16
#define RGB_grey74    189  |  189 <<8  |  189 <<16
#define RGB_grey    190  |  190 <<8  |  190 <<16
#define RGB_grey75    191  |  191 <<8  |  191 <<16
#define RGB_grey76    194  |  194 <<8  |  194 <<16
#define RGB_grey77    196  |  196 <<8  |  196 <<16
#define RGB_grey78    199  |  199 <<8  |  199 <<16
#define RGB_grey79    201  |  201 <<8  |  201 <<16
#define RGB_grey80    204  |  204 <<8  |  204 <<16
#define RGB_grey81    207  |  207 <<8  |  207 <<16
#define RGB_grey82    209  |  209 <<8  |  209 <<16
#define RGB_lightgrey    211  |  211 <<8  |  211 <<16
#define RGB_grey83    212  |  212 <<8  |  212 <<16
#define RGB_grey84    214  |  214 <<8  |  214 <<16
#define RGB_grey85    217  |  217 <<8  |  217 <<16
#define RGB_grey86    219  |  219 <<8  |  219 <<16
#define RGB_grey87    222  |  222 <<8  |  222 <<16
#define RGB_grey88    224  |  224 <<8  |  224 <<16
#define RGB_grey89    227  |  227 <<8  |  227 <<16
#define RGB_grey90    229  |  229 <<8  |  229 <<16
#define RGB_grey91    232  |  232 <<8  |  232 <<16
#define RGB_grey92    235  |  235 <<8  |  235 <<16
#define RGB_grey93    237  |  237 <<8  |  237 <<16
#define RGB_grey94    240  |  240 <<8  |  240 <<16
#define RGB_grey95    242  |  242 <<8  |  242 <<16
#define RGB_grey96    245  |  245 <<8  |  245 <<16
#define RGB_grey97    247  |  247 <<8  |  247 <<16
#define RGB_grey98    250  |  250 <<8  |  250 <<16
#define RGB_grey99    252  |  252 <<8  |  252 <<16
#define RGB_grey100    255  |  255 <<8  |  255 <<16
#define RGB_rainbow0    255  |  0 <<8  |  0 <<16
#define RGB_rainbow1    255  |  165 <<8  |  0 <<16
#define RGB_rainbow2    240  |  235 <<8  |  0 <<16
#define RGB_rainbow3    0  |  128 <<8  |  0 <<16
#define RGB_rainbow4    0  |  0 <<8  |  255 <<16
#define RGB_rainbow5    75  |  0 <<8  |  130 <<16
#define RGB_rainbow6    238  |  130 <<8  |  238 <<16
#define RGB_rainbow7    0  |  0 <<8  |  0 <<16




#define CF_ghostwhite    RGB2FLOAT(248,   248,   255)
#define CF_whitesmoke    RGB2FLOAT(245,   245,   245)
#define CF_gainsboro    RGB2FLOAT(220,   220,   220)
#define CF_floralwhite    RGB2FLOAT(255,   250,   240)
#define CF_oldlace    RGB2FLOAT(253,   245,   230)
#define CF_linen    RGB2FLOAT(250,   240,   230)
#define CF_antiquewhite    RGB2FLOAT(250,   235,   215)
#define CF_papayawhip    RGB2FLOAT(255,   239,   213)
#define CF_blanchedalmond    RGB2FLOAT(255,   235,   205)
#define CF_mintcream    RGB2FLOAT(245,   255,   250)
#define CF_aliceblue    RGB2FLOAT(240,   248,   255)
#define CF_lavender    RGB2FLOAT(230,   230,   250)
#define CF_lavenderblush    RGB2FLOAT(255,   240,   245)
#define CF_lavenderblush1    RGB2FLOAT(255,   240,   245)
#define CF_snow    RGB2FLOAT(255,   250,   250)
#define CF_snow1    RGB2FLOAT(255,   250,   250)
#define CF_snow2    RGB2FLOAT(238,   233,   233)
#define CF_snow3    RGB2FLOAT(205,   201,   201)
#define CF_snow4    RGB2FLOAT(139,   137,   137)
#define CF_seashell    RGB2FLOAT(255,   245,   238)
#define CF_seashell1    RGB2FLOAT(255,   245,   238)
#define CF_seashell2    RGB2FLOAT(238,   229,   222)
#define CF_seashell3    RGB2FLOAT(205,   197,   191)
#define CF_seashell4    RGB2FLOAT(139,   134,   130)
#define CF_antiquewhite1    RGB2FLOAT(255,   239,   219)
#define CF_antiquewhite2    RGB2FLOAT(238,   223,   204)
#define CF_antiquewhite3    RGB2FLOAT(205,   192,   176)
#define CF_antiquewhite4    RGB2FLOAT(139,   131,   120)
#define CF_bisque    RGB2FLOAT(255,   228,   196)
#define CF_bisque1    RGB2FLOAT(255,   228,   196)
#define CF_bisque2    RGB2FLOAT(238,   213,   183)
#define CF_bisque3    RGB2FLOAT(205,   183,   158)
#define CF_bisque4    RGB2FLOAT(139,   125,   107)
#define CF_peachpuff    RGB2FLOAT(255,   218,   185)
#define CF_peachpuff1    RGB2FLOAT(255,   218,   185)
#define CF_peachpuff2    RGB2FLOAT(238,   203,   173)
#define CF_peachpuff3    RGB2FLOAT(205,   175,   149)
#define CF_peachpuff4    RGB2FLOAT(139,   119,   101)
#define CF_navajowhite    RGB2FLOAT(255,   222,   173)
#define CF_navajowhite1    RGB2FLOAT(255,   222,   173)
#define CF_navajowhite2    RGB2FLOAT(238,   207,   161)
#define CF_navajowhite3    RGB2FLOAT(205,   179,   139)
#define CF_navajowhite4    RGB2FLOAT(139,   121,   94)
#define CF_moccasin    RGB2FLOAT(255,   228,   181)
#define CF_lemonchiffon    RGB2FLOAT(255,   250,   205)
#define CF_lemonchiffon1    RGB2FLOAT(255,   250,   205)
#define CF_lemonchiffon2    RGB2FLOAT(238,   233,   191)
#define CF_lemonchiffon3    RGB2FLOAT(205,   201,   165)
#define CF_lemonchiffon4    RGB2FLOAT(139,   137,   112)
#define CF_beige    RGB2FLOAT(245,   245,   220)
#define CF_cornsilk    RGB2FLOAT(255,   248,   220)
#define CF_cornsilk1    RGB2FLOAT(255,   248,   220)
#define CF_cornsilk2    RGB2FLOAT(238,   232,   205)
#define CF_cornsilk3    RGB2FLOAT(205,   200,   177)
#define CF_cornsilk4    RGB2FLOAT(139,   136,   120)
#define CF_ivory    RGB2FLOAT(255,   255,   240)
#define CF_ivory1    RGB2FLOAT(255,   255,   240)
#define CF_ivory2    RGB2FLOAT(238,   238,   224)
#define CF_ivory3    RGB2FLOAT(205,   205,   193)
#define CF_ivory4    RGB2FLOAT(139,   139,   131)
#define CF_honeydew    RGB2FLOAT(240,   255,   240)
#define CF_honeydew1    RGB2FLOAT(240,   255,   240)
#define CF_honeydew2    RGB2FLOAT(224,   238,   224)
#define CF_honeydew3    RGB2FLOAT(193,   205,   193)
#define CF_honeydew4    RGB2FLOAT(131,   139,   131)
#define CF_lavenderblush2    RGB2FLOAT(238,   224,   229)
#define CF_lavenderblush3    RGB2FLOAT(205,   193,   197)
#define CF_lavenderblush4    RGB2FLOAT(139,   131,   134)
#define CF_mistyrose    RGB2FLOAT(255,   228,   225)
#define CF_mistyrose1    RGB2FLOAT(255,   228,   225)
#define CF_mistyrose2    RGB2FLOAT(238,   213,   210)
#define CF_mistyrose3    RGB2FLOAT(205,   183,   181)
#define CF_mistyrose4    RGB2FLOAT(139,   125,   123)
#define CF_azure    RGB2FLOAT(240,   255,   255)
#define CF_azure1    RGB2FLOAT(240,   255,   255)
#define CF_azure2    RGB2FLOAT(224,   238,   238)
#define CF_azure3    RGB2FLOAT(193,   205,   205)
#define CF_azure4    RGB2FLOAT(131,   139,   139)
#define CF_lightslateblue    RGB2FLOAT(132,   112,   255)
#define CF_mediumslateblue    RGB2FLOAT(123,   104,   238)
#define CF_darkslateblue    RGB2FLOAT(72,   61,   139)
#define CF_slateblue    RGB2FLOAT(106,   90,   205)
#define CF_slateblue1    RGB2FLOAT(131,   111,   255)
#define CF_slateblue2    RGB2FLOAT(122,   103,   238)
#define CF_slateblue3    RGB2FLOAT(105,   89,   205)
#define CF_slateblue4    RGB2FLOAT(71,   60,   139)
#define CF_royalblue    RGB2FLOAT(65,   105,   225)
#define CF_royalblue1    RGB2FLOAT(72,   118,   255)
#define CF_royalblue2    RGB2FLOAT(67,   110,   238)
#define CF_royalblue3    RGB2FLOAT(58,   95,   205)
#define CF_royalblue4    RGB2FLOAT(39,   64,   139)
#define CF_blue    RGB2FLOAT(0,   0,   255)
#define CF_blue1    RGB2FLOAT(0,   0,   255)
#define CF_blue2    RGB2FLOAT(0,   0,   238)
#define CF_blue3    RGB2FLOAT(0,   0,   205)
#define CF_blue4    RGB2FLOAT(0,   0,   139)
#define CF_navy    RGB2FLOAT(0,   0,   128)
#define CF_navyblue    RGB2FLOAT(0,   0,   128)
#define CF_darkblue    RGB2FLOAT(0,   0,   139)
#define CF_midnightblue    RGB2FLOAT(25,   25,   112)
#define CF_dodgerblue    RGB2FLOAT(30,   144,   255)
#define CF_dodgerblue1    RGB2FLOAT(30,   144,   255)
#define CF_dodgerblue2    RGB2FLOAT(28,   134,   238)
#define CF_dodgerblue3    RGB2FLOAT(24,   116,   205)
#define CF_dodgerblue4    RGB2FLOAT(16,   78,   139)
#define CF_steelblue    RGB2FLOAT(70,   130,   180)
#define CF_steelblue1    RGB2FLOAT(99,   184,   255)
#define CF_steelblue2    RGB2FLOAT(92,   172,   238)
#define CF_steelblue3    RGB2FLOAT(79,   148,   205)
#define CF_steelblue4    RGB2FLOAT(54,   100,   139)
#define CF_deepskyblue    RGB2FLOAT(0,   191,   255)
#define CF_deepskyblue1    RGB2FLOAT(0,   191,   255)
#define CF_deepskyblue2    RGB2FLOAT(0,   178,   238)
#define CF_deepskyblue3    RGB2FLOAT(0,   154,   205)
#define CF_deepskyblue4    RGB2FLOAT(0,   104,   139)
#define CF_skyblue    RGB2FLOAT(135,   206,   235)
#define CF_skyblue1    RGB2FLOAT(135,   206,   255)
#define CF_skyblue2    RGB2FLOAT(126,   192,   238)
#define CF_skyblue3    RGB2FLOAT(108,   166,   205)
#define CF_skyblue4    RGB2FLOAT(74,   112,   139)
#define CF_cornflowerblue    RGB2FLOAT(100,   149,   237)
#define CF_mediumblue    RGB2FLOAT(0,   0,   205)
#define CF_lightskyblue    RGB2FLOAT(135,   206,   250)
#define CF_lightskyblue1    RGB2FLOAT(176,   226,   255)
#define CF_lightskyblue2    RGB2FLOAT(164,   211,   238)
#define CF_lightskyblue3    RGB2FLOAT(141,   182,   205)
#define CF_lightskyblue4    RGB2FLOAT(96,   123,   139)
#define CF_lightslategray    RGB2FLOAT(119,   136,   153)
#define CF_lightslategrey    RGB2FLOAT(119,   136,   153)
#define CF_slategray    RGB2FLOAT(112,   128,   144)
#define CF_slategrey    RGB2FLOAT(112,   128,   144)
#define CF_slategray1    RGB2FLOAT(198,   226,   255)
#define CF_slategray2    RGB2FLOAT(185,   211,   238)
#define CF_slategray3    RGB2FLOAT(159,   182,   205)
#define CF_slategray4    RGB2FLOAT(108,   123,   139)
#define CF_lightsteelblue1    RGB2FLOAT(202,   225,   255)
#define CF_lightsteelblue2    RGB2FLOAT(188,   210,   238)
#define CF_lightsteelblue3    RGB2FLOAT(162,   181,   205)
#define CF_lightsteelblue4    RGB2FLOAT(110,   123,   139)
#define CF_lightblue1    RGB2FLOAT(191,   239,   255)
#define CF_lightblue2    RGB2FLOAT(178,   223,   238)
#define CF_lightblue3    RGB2FLOAT(154,   192,   205)
#define CF_lightblue4    RGB2FLOAT(104,   131,   139)
#define CF_lightcyan    RGB2FLOAT(224,   255,   255)
#define CF_lightcyan1    RGB2FLOAT(224,   255,   255)
#define CF_lightcyan2    RGB2FLOAT(209,   238,   238)
#define CF_lightcyan3    RGB2FLOAT(180,   205,   205)
#define CF_lightcyan4    RGB2FLOAT(122,   139,   139)
#define CF_paleturquoise1    RGB2FLOAT(187,   255,   255)
#define CF_paleturquoise2    RGB2FLOAT(174,   238,   238)
#define CF_paleturquoise3    RGB2FLOAT(150,   205,   205)
#define CF_paleturquoise4    RGB2FLOAT(102,   139,   139)
#define CF_cadetblue1    RGB2FLOAT(152,   245,   255)
#define CF_cadetblue2    RGB2FLOAT(142,   229,   238)
#define CF_cadetblue3    RGB2FLOAT(122,   197,   205)
#define CF_cadetblue4    RGB2FLOAT(83,   134,   139)
#define CF_turquoise1    RGB2FLOAT(0,   245,   255)
#define CF_turquoise2    RGB2FLOAT(0,   229,   238)
#define CF_turquoise3    RGB2FLOAT(0,   197,   205)
#define CF_turquoise4    RGB2FLOAT(0,   134,   139)
#define CF_lightsteelblue    RGB2FLOAT(176,   196,   222)
#define CF_lightblue    RGB2FLOAT(173,   216,   230)
#define CF_powderblue    RGB2FLOAT(176,   224,   230)
#define CF_paleturquoise    RGB2FLOAT(175,   238,   238)
#define CF_darkturquoise    RGB2FLOAT(0,   206,   209)
#define CF_mediumturquoise    RGB2FLOAT(72,   209,   204)
#define CF_turquoise    RGB2FLOAT(64,   224,   208)
#define CF_cadetblue    RGB2FLOAT(95,   158,   160)
#define CF_lightgreen    RGB2FLOAT(144,   238,   144)
#define CF_darkgreen    RGB2FLOAT(0,   100,   0)
#define CF_darkolivegreen    RGB2FLOAT(85,   107,   47)
#define CF_palegreen    RGB2FLOAT(152,   251,   152)
#define CF_lawngreen    RGB2FLOAT(124,   252,   0)
#define CF_greenyellow    RGB2FLOAT(173,   255,   47)
#define CF_limegreen    RGB2FLOAT(50,   205,   50)
#define CF_yellowgreen    RGB2FLOAT(154,   205,   50)
#define CF_forestgreen    RGB2FLOAT(34,   139,   34)
#define CF_cyan    RGB2FLOAT(0,   255,   255)
#define CF_cyan1    RGB2FLOAT(0,   255,   255)
#define CF_cyan2    RGB2FLOAT(0,   238,   238)
#define CF_cyan3    RGB2FLOAT(0,   205,   205)
#define CF_cyan4    RGB2FLOAT(0,   139,   139)
#define CF_darkcyan    RGB2FLOAT(0,   139,   139)
#define CF_darkslategray1    RGB2FLOAT(151,   255,   255)
#define CF_darkslategray2    RGB2FLOAT(141,   238,   238)
#define CF_darkslategray3    RGB2FLOAT(121,   205,   205)
#define CF_darkslategray4    RGB2FLOAT(82,   139,   139)
#define CF_darkslategray    RGB2FLOAT(47,   79,   79)
#define CF_darkslategrey    RGB2FLOAT(47,   79,   79)
#define CF_aquamarine    RGB2FLOAT(127,   255,   212)
#define CF_aquamarine1    RGB2FLOAT(127,   255,   212)
#define CF_aquamarine2    RGB2FLOAT(118,   238,   198)
#define CF_aquamarine3    RGB2FLOAT(102,   205,   170)
#define CF_aquamarine4    RGB2FLOAT(69,   139,   116)
#define CF_mediumaquamarine    RGB2FLOAT(102,   205,   170)
#define CF_mediumseagreen    RGB2FLOAT(60,   179,   113)
#define CF_lightseagreen    RGB2FLOAT(32,   178,   170)
#define CF_seagreen    RGB2FLOAT(46,   139,   87)
#define CF_seagreen1    RGB2FLOAT(84,   255,   159)
#define CF_seagreen2    RGB2FLOAT(78,   238,   148)
#define CF_seagreen3    RGB2FLOAT(67,   205,   128)
#define CF_seagreen4    RGB2FLOAT(46,   139,   87)
#define CF_darkseagreen    RGB2FLOAT(143,   188,   143)
#define CF_darkseagreen1    RGB2FLOAT(193,   255,   193)
#define CF_darkseagreen2    RGB2FLOAT(180,   238,   180)
#define CF_darkseagreen3    RGB2FLOAT(155,   205,   155)
#define CF_darkseagreen4    RGB2FLOAT(105,   139,   105)
#define CF_palegreen1    RGB2FLOAT(154,   255,   154)
#define CF_palegreen2    RGB2FLOAT(144,   238,   144)
#define CF_palegreen3    RGB2FLOAT(124,   205,   124)
#define CF_palegreen4    RGB2FLOAT(84,   139,   84)
#define CF_mediumspringgreen    RGB2FLOAT(0,   250,   154)
#define CF_springgreen    RGB2FLOAT(0,   255,   127)
#define CF_springgreen1    RGB2FLOAT(0,   255,   127)
#define CF_springgreen2    RGB2FLOAT(0,   238,   118)
#define CF_springgreen3    RGB2FLOAT(0,   205,   102)
#define CF_springgreen4    RGB2FLOAT(0,   139,   69)
#define CF_green    RGB2FLOAT(0,   255,   0)
#define CF_green1    RGB2FLOAT(0,   255,   0)
#define CF_green2    RGB2FLOAT(0,   238,   0)
#define CF_green3    RGB2FLOAT(0,   205,   0)
#define CF_green4    RGB2FLOAT(0,   139,   0)
#define CF_chartreuse    RGB2FLOAT(127,   255,   0)
#define CF_chartreuse1    RGB2FLOAT(127,   255,   0)
#define CF_chartreuse2    RGB2FLOAT(118,   238,   0)
#define CF_chartreuse3    RGB2FLOAT(102,   205,   0)
#define CF_chartreuse4    RGB2FLOAT(69,   139,   0)
#define CF_olivedrab    RGB2FLOAT(107,   142,   35)
#define CF_olivedrab1    RGB2FLOAT(192,   255,   62)
#define CF_olivedrab2    RGB2FLOAT(179,   238,   58)
#define CF_olivedrab3    RGB2FLOAT(154,   205,   50)
#define CF_olivedrab4    RGB2FLOAT(105,   139,   34)
#define CF_darkolivegreen1    RGB2FLOAT(202,   255,   112)
#define CF_darkolivegreen2    RGB2FLOAT(188,   238,   104)
#define CF_darkolivegreen3    RGB2FLOAT(162,   205,   90)
#define CF_darkolivegreen4    RGB2FLOAT(110,   139,   61)
#define CF_darkkhaki    RGB2FLOAT(189,   183,   107)
#define CF_khaki    RGB2FLOAT(240,   230,   140)
#define CF_khaki1    RGB2FLOAT(255,   246,   143)
#define CF_khaki2    RGB2FLOAT(238,   230,   133)
#define CF_khaki3    RGB2FLOAT(205,   198,   115)
#define CF_khaki4    RGB2FLOAT(139,   134,   78)
#define CF_lightyellow    RGB2FLOAT(255,   255,   224)
#define CF_lightyellow1    RGB2FLOAT(255,   255,   224)
#define CF_lightyellow2    RGB2FLOAT(238,   238,   209)
#define CF_lightyellow3    RGB2FLOAT(205,   205,   180)
#define CF_lightyellow4    RGB2FLOAT(139,   139,   122)
#define CF_yellow    RGB2FLOAT(255,   255,   0)
#define CF_yellow1    RGB2FLOAT(255,   255,   0)
#define CF_yellow2    RGB2FLOAT(238,   238,   0)
#define CF_yellow3    RGB2FLOAT(205,   205,   0)
#define CF_yellow4    RGB2FLOAT(139,   139,   0)
#define CF_gold    RGB2FLOAT(255,   215,   0)
#define CF_gold1    RGB2FLOAT(255,   215,   0)
#define CF_gold2    RGB2FLOAT(238,   201,   0)
#define CF_gold3    RGB2FLOAT(205,   173,   0)
#define CF_gold4    RGB2FLOAT(139,   117,   0)
#define CF_goldenrod    RGB2FLOAT(218,   165,   32)
#define CF_goldenrod1    RGB2FLOAT(255,   193,   37)
#define CF_goldenrod2    RGB2FLOAT(238,   180,   34)
#define CF_goldenrod3    RGB2FLOAT(205,   155,   29)
#define CF_goldenrod4    RGB2FLOAT(139,   105,   20)
#define CF_lightgoldenrodyellow    RGB2FLOAT(250,   250,   210)
#define CF_lightgoldenrod    RGB2FLOAT(238,   221,   130)
#define CF_lightgoldenrod1    RGB2FLOAT(255,   236,   139)
#define CF_lightgoldenrod2    RGB2FLOAT(238,   220,   130)
#define CF_lightgoldenrod3    RGB2FLOAT(205,   190,   112)
#define CF_lightgoldenrod4    RGB2FLOAT(139,   129,   76)
#define CF_palegoldenrod    RGB2FLOAT(238,   232,   170)
#define CF_darkgoldenrod    RGB2FLOAT(184,   134,   11)
#define CF_darkgoldenrod1    RGB2FLOAT(255,   185,   15)
#define CF_darkgoldenrod2    RGB2FLOAT(238,   173,   14)
#define CF_darkgoldenrod3    RGB2FLOAT(205,   149,   12)
#define CF_darkgoldenrod4    RGB2FLOAT(139,   101,   8)
#define CF_rosybrown    RGB2FLOAT(188,   143,   143)
#define CF_rosybrown1    RGB2FLOAT(255,   193,   193)
#define CF_rosybrown2    RGB2FLOAT(238,   180,   180)
#define CF_rosybrown3    RGB2FLOAT(205,   155,   155)
#define CF_rosybrown4    RGB2FLOAT(139,   105,   105)
#define CF_indianred    RGB2FLOAT(205,   92,   92)
#define CF_indianred1    RGB2FLOAT(255,   106,   106)
#define CF_indianred2    RGB2FLOAT(238,   99,   99)
#define CF_indianred3    RGB2FLOAT(205,   85,   85)
#define CF_indianred4    RGB2FLOAT(139,   58,   58)
#define CF_saddlebrown    RGB2FLOAT(139,   69,   19)
#define CF_sandybrown    RGB2FLOAT(244,   164,   96)
#define CF_peru    RGB2FLOAT(205,   133,   63)
#define CF_sienna    RGB2FLOAT(160,   82,   45)
#define CF_sienna1    RGB2FLOAT(255,   130,   71)
#define CF_sienna2    RGB2FLOAT(238,   121,   66)
#define CF_sienna3    RGB2FLOAT(205,   104,   57)
#define CF_sienna4    RGB2FLOAT(139,   71,   38)
#define CF_burlywood    RGB2FLOAT(222,   184,   135)
#define CF_burlywood1    RGB2FLOAT(255,   211,   155)
#define CF_burlywood2    RGB2FLOAT(238,   197,   145)
#define CF_burlywood3    RGB2FLOAT(205,   170,   125)
#define CF_burlywood4    RGB2FLOAT(139,   115,   85)
#define CF_wheat    RGB2FLOAT(245,   222,   179)
#define CF_wheat1    RGB2FLOAT(255,   231,   186)
#define CF_wheat2    RGB2FLOAT(238,   216,   174)
#define CF_wheat3    RGB2FLOAT(205,   186,   150)
#define CF_wheat4    RGB2FLOAT(139,   126,   102)
#define CF_tan    RGB2FLOAT(210,   180,   140)
#define CF_tan1    RGB2FLOAT(255,   165,   79)
#define CF_tan2    RGB2FLOAT(238,   154,   73)
#define CF_tan3    RGB2FLOAT(205,   133,   63)
#define CF_tan4    RGB2FLOAT(139,   90,   43)
#define CF_chocolate    RGB2FLOAT(210,   105,   30)
#define CF_chocolate1    RGB2FLOAT(255,   127,   36)
#define CF_chocolate2    RGB2FLOAT(238,   118,   33)
#define CF_chocolate3    RGB2FLOAT(205,   102,   29)
#define CF_chocolate4    RGB2FLOAT(139,   69,   19)
#define CF_firebrick    RGB2FLOAT(178,   34,   34)
#define CF_firebrick1    RGB2FLOAT(255,   48,   48)
#define CF_firebrick2    RGB2FLOAT(238,   44,   44)
#define CF_firebrick3    RGB2FLOAT(205,   38,   38)
#define CF_firebrick4    RGB2FLOAT(139,   26,   26)
#define CF_brown    RGB2FLOAT(165,   42,   42)
#define CF_brown1    RGB2FLOAT(255,   64,   64)
#define CF_brown2    RGB2FLOAT(238,   59,   59)
#define CF_brown3    RGB2FLOAT(205,   51,   51)
#define CF_brown4    RGB2FLOAT(139,   35,   35)
#define CF_salmon    RGB2FLOAT(250,   128,   114)
#define CF_salmon1    RGB2FLOAT(255,   140,   105)
#define CF_salmon2    RGB2FLOAT(238,   130,   98)
#define CF_salmon3    RGB2FLOAT(205,   112,   84)
#define CF_salmon4    RGB2FLOAT(139,   76,   57)
#define CF_darksalmon    RGB2FLOAT(233,   150,   122)
#define CF_lightsalmon    RGB2FLOAT(255,   160,   122)
#define CF_lightsalmon1    RGB2FLOAT(255,   160,   122)
#define CF_lightsalmon2    RGB2FLOAT(238,   149,   114)
#define CF_lightsalmon3    RGB2FLOAT(205,   129,   98)
#define CF_lightsalmon4    RGB2FLOAT(139,   87,   66)
#define CF_orange    RGB2FLOAT(255,   165,   0)
#define CF_orange1    RGB2FLOAT(255,   165,   0)
#define CF_orange2    RGB2FLOAT(238,   154,   0)
#define CF_orange3    RGB2FLOAT(205,   133,   0)
#define CF_orange4    RGB2FLOAT(139,   90,   0)
#define CF_darkorange    RGB2FLOAT(255,   140,   0)
#define CF_darkorange1    RGB2FLOAT(255,   127,   0)
#define CF_darkorange2    RGB2FLOAT(238,   118,   0)
#define CF_darkorange3    RGB2FLOAT(205,   102,   0)
#define CF_darkorange4    RGB2FLOAT(139,   69,   0)
#define CF_lightcoral    RGB2FLOAT(240,   128,   128)
#define CF_coral    RGB2FLOAT(255,   127,   80)
#define CF_coral1    RGB2FLOAT(255,   114,   86)
#define CF_coral2    RGB2FLOAT(238,   106,   80)
#define CF_coral3    RGB2FLOAT(205,   91,   69)
#define CF_coral4    RGB2FLOAT(139,   62,   47)
#define CF_tomato    RGB2FLOAT(255,   99,   71)
#define CF_tomato1    RGB2FLOAT(255,   99,   71)
#define CF_tomato2    RGB2FLOAT(238,   92,   66)
#define CF_tomato3    RGB2FLOAT(205,   79,   57)
#define CF_tomato4    RGB2FLOAT(139,   54,   38)
#define CF_orangered    RGB2FLOAT(255,   69,   0)
#define CF_orangered1    RGB2FLOAT(255,   69,   0)
#define CF_orangered2    RGB2FLOAT(238,   64,   0)
#define CF_orangered3    RGB2FLOAT(205,   55,   0)
#define CF_orangered4    RGB2FLOAT(139,   37,   0)
#define CF_red    RGB2FLOAT(255,   0,   0)
#define CF_red1    RGB2FLOAT(255,   0,   0)
#define CF_red2    RGB2FLOAT(238,   0,   0)
#define CF_red3    RGB2FLOAT(205,   0,   0)
#define CF_red4    RGB2FLOAT(139,   0,   0)
#define CF_darkred    RGB2FLOAT(139,   0,   0)
#define CF_deeppink    RGB2FLOAT(255,   20,   147)
#define CF_deeppink1    RGB2FLOAT(255,   20,   147)
#define CF_deeppink2    RGB2FLOAT(238,   18,   137)
#define CF_deeppink3    RGB2FLOAT(205,   16,   118)
#define CF_deeppink4    RGB2FLOAT(139,   10,   80)
#define CF_hotpink    RGB2FLOAT(255,   105,   180)
#define CF_hotpink1    RGB2FLOAT(255,   110,   180)
#define CF_hotpink2    RGB2FLOAT(238,   106,   167)
#define CF_hotpink3    RGB2FLOAT(205,   96,   144)
#define CF_hotpink4    RGB2FLOAT(139,   58,   98)
#define CF_pink    RGB2FLOAT(255,   192,   203)
#define CF_pink1    RGB2FLOAT(255,   181,   197)
#define CF_pink2    RGB2FLOAT(238,   169,   184)
#define CF_pink3    RGB2FLOAT(205,   145,   158)
#define CF_pink4    RGB2FLOAT(139,   99,   108)
#define CF_lightpink    RGB2FLOAT(255,   182,   193)
#define CF_lightpink1    RGB2FLOAT(255,   174,   185)
#define CF_lightpink2    RGB2FLOAT(238,   162,   173)
#define CF_lightpink3    RGB2FLOAT(205,   140,   149)
#define CF_lightpink4    RGB2FLOAT(139,   95,   101)
#define CF_palevioletred    RGB2FLOAT(219,   112,   147)
#define CF_palevioletred1    RGB2FLOAT(255,   130,   171)
#define CF_palevioletred2    RGB2FLOAT(238,   121,   159)
#define CF_palevioletred3    RGB2FLOAT(205,   104,   137)
#define CF_palevioletred4    RGB2FLOAT(139,   71,   93)
#define CF_maroon    RGB2FLOAT(176,   48,   96)
#define CF_maroon1    RGB2FLOAT(255,   52,   179)
#define CF_maroon2    RGB2FLOAT(238,   48,   167)
#define CF_maroon3    RGB2FLOAT(205,   41,   144)
#define CF_maroon4    RGB2FLOAT(139,   28,   98)
#define CF_mediumvioletred    RGB2FLOAT(199,   21,   133)
#define CF_violet    RGB2FLOAT(238,   130,   238)
#define CF_violetred    RGB2FLOAT(208,   32,   144)
#define CF_violetred1    RGB2FLOAT(255,   62,   150)
#define CF_violetred2    RGB2FLOAT(238,   58,   140)
#define CF_violetred3    RGB2FLOAT(205,   50,   120)
#define CF_violetred4    RGB2FLOAT(139,   34,   82)
#define CF_magenta    RGB2FLOAT(255,   0,   255)
#define CF_magenta1    RGB2FLOAT(255,   0,   255)
#define CF_magenta2    RGB2FLOAT(238,   0,   238)
#define CF_magenta3    RGB2FLOAT(205,   0,   205)
#define CF_magenta4    RGB2FLOAT(139,   0,   139)
#define CF_darkmagenta    RGB2FLOAT(139,   0,   139)
#define CF_orchid    RGB2FLOAT(218,   112,   214)
#define CF_orchid1    RGB2FLOAT(255,   131,   250)
#define CF_orchid2    RGB2FLOAT(238,   122,   233)
#define CF_orchid3    RGB2FLOAT(205,   105,   201)
#define CF_orchid4    RGB2FLOAT(139,   71,   137)
#define CF_plum    RGB2FLOAT(221,   160,   221)
#define CF_plum1    RGB2FLOAT(255,   187,   255)
#define CF_plum2    RGB2FLOAT(238,   174,   238)
#define CF_plum3    RGB2FLOAT(205,   150,   205)
#define CF_plum4    RGB2FLOAT(139,   102,   139)
#define CF_mediumorchid    RGB2FLOAT(186,   85,   211)
#define CF_mediumorchid1    RGB2FLOAT(224,   102,   255)
#define CF_mediumorchid2    RGB2FLOAT(209,   95,   238)
#define CF_mediumorchid3    RGB2FLOAT(180,   82,   205)
#define CF_mediumorchid4    RGB2FLOAT(122,   55,   139)
#define CF_darkorchid    RGB2FLOAT(153,   50,   204)
#define CF_darkorchid1    RGB2FLOAT(191,   62,   255)
#define CF_darkorchid2    RGB2FLOAT(178,   58,   238)
#define CF_darkorchid3    RGB2FLOAT(154,   50,   205)
#define CF_darkorchid4    RGB2FLOAT(104,   34,   139)
#define CF_purple    RGB2FLOAT(160,   32,   240)
#define CF_purple1    RGB2FLOAT(155,   48,   255)
#define CF_purple2    RGB2FLOAT(145,   44,   238)
#define CF_purple3    RGB2FLOAT(125,   38,   205)
#define CF_purple4    RGB2FLOAT(85,   26,   139)
#define CF_mediumpurple1    RGB2FLOAT(171,   130,   255)
#define CF_mediumpurple2    RGB2FLOAT(159,   121,   238)
#define CF_mediumpurple3    RGB2FLOAT(137,   104,   205)
#define CF_mediumpurple4    RGB2FLOAT(93,   71,   139)
#define CF_thistle    RGB2FLOAT(216,   191,   216)
#define CF_thistle1    RGB2FLOAT(255,   225,   255)
#define CF_thistle2    RGB2FLOAT(238,   210,   238)
#define CF_thistle3    RGB2FLOAT(205,   181,   205)
#define CF_thistle4    RGB2FLOAT(139,   123,   139)
#define CF_darkviolet    RGB2FLOAT(148,   0,   211)
#define CF_blueviolet    RGB2FLOAT(138,   43,   226)
#define CF_mediumpurple    RGB2FLOAT(147,   112,   219)
#define CF_black    RGB2FLOAT(0,   0,   0)
#define CF_gray0    RGB2FLOAT(0,   0,   0)
#define CF_gray1    RGB2FLOAT(3,   3,   3)
#define CF_gray2    RGB2FLOAT(5,   5,   5)
#define CF_gray3    RGB2FLOAT(8,   8,   8)
#define CF_gray4    RGB2FLOAT(10,   10,   10)
#define CF_gray5    RGB2FLOAT(13,   13,   13)
#define CF_gray6    RGB2FLOAT(15,   15,   15)
#define CF_gray7    RGB2FLOAT(18,   18,   18)
#define CF_gray8    RGB2FLOAT(20,   20,   20)
#define CF_gray9    RGB2FLOAT(23,   23,   23)
#define CF_gray10    RGB2FLOAT(26,   26,   26)
#define CF_gray11    RGB2FLOAT(28,   28,   28)
#define CF_gray12    RGB2FLOAT(31,   31,   31)
#define CF_gray13    RGB2FLOAT(33,   33,   33)
#define CF_gray14    RGB2FLOAT(36,   36,   36)
#define CF_gray15    RGB2FLOAT(38,   38,   38)
#define CF_gray16    RGB2FLOAT(41,   41,   41)
#define CF_gray17    RGB2FLOAT(43,   43,   43)
#define CF_gray18    RGB2FLOAT(46,   46,   46)
#define CF_gray19    RGB2FLOAT(48,   48,   48)
#define CF_gray20    RGB2FLOAT(51,   51,   51)
#define CF_gray21    RGB2FLOAT(54,   54,   54)
#define CF_gray22    RGB2FLOAT(56,   56,   56)
#define CF_gray23    RGB2FLOAT(59,   59,   59)
#define CF_gray24    RGB2FLOAT(61,   61,   61)
#define CF_gray25    RGB2FLOAT(64,   64,   64)
#define CF_gray26    RGB2FLOAT(66,   66,   66)
#define CF_gray27    RGB2FLOAT(69,   69,   69)
#define CF_gray28    RGB2FLOAT(71,   71,   71)
#define CF_gray29    RGB2FLOAT(74,   74,   74)
#define CF_gray30    RGB2FLOAT(77,   77,   77)
#define CF_gray31    RGB2FLOAT(79,   79,   79)
#define CF_gray32    RGB2FLOAT(82,   82,   82)
#define CF_gray33    RGB2FLOAT(84,   84,   84)
#define CF_gray34    RGB2FLOAT(87,   87,   87)
#define CF_gray35    RGB2FLOAT(89,   89,   89)
#define CF_gray36    RGB2FLOAT(92,   92,   92)
#define CF_gray37    RGB2FLOAT(94,   94,   94)
#define CF_gray38    RGB2FLOAT(97,   97,   97)
#define CF_gray39    RGB2FLOAT(99,   99,   99)
#define CF_gray40    RGB2FLOAT(102,   102,   102)
#define CF_gray41    RGB2FLOAT(105,   105,   105)
#define CF_dimgray    RGB2FLOAT(105,   105,   105)
#define CF_gray42    RGB2FLOAT(107,   107,   107)
#define CF_gray43    RGB2FLOAT(110,   110,   110)
#define CF_gray44    RGB2FLOAT(112,   112,   112)
#define CF_gray45    RGB2FLOAT(115,   115,   115)
#define CF_gray46    RGB2FLOAT(117,   117,   117)
#define CF_gray47    RGB2FLOAT(120,   120,   120)
#define CF_gray48    RGB2FLOAT(122,   122,   122)
#define CF_gray49    RGB2FLOAT(125,   125,   125)
#define CF_gray50    RGB2FLOAT(127,   127,   127)
#define CF_gray51    RGB2FLOAT(130,   130,   130)
#define CF_gray52    RGB2FLOAT(133,   133,   133)
#define CF_gray53    RGB2FLOAT(135,   135,   135)
#define CF_gray54    RGB2FLOAT(138,   138,   138)
#define CF_gray55    RGB2FLOAT(140,   140,   140)
#define CF_gray56    RGB2FLOAT(143,   143,   143)
#define CF_gray57    RGB2FLOAT(145,   145,   145)
#define CF_gray58    RGB2FLOAT(148,   148,   148)
#define CF_gray59    RGB2FLOAT(150,   150,   150)
#define CF_gray60    RGB2FLOAT(153,   153,   153)
#define CF_gray61    RGB2FLOAT(156,   156,   156)
#define CF_gray62    RGB2FLOAT(158,   158,   158)
#define CF_gray63    RGB2FLOAT(161,   161,   161)
#define CF_gray64    RGB2FLOAT(163,   163,   163)
#define CF_gray65    RGB2FLOAT(166,   166,   166)
#define CF_darkgray    RGB2FLOAT(169,   169,   169)
#define CF_gray66    RGB2FLOAT(168,   168,   168)
#define CF_gray67    RGB2FLOAT(171,   171,   171)
#define CF_gray68    RGB2FLOAT(173,   173,   173)
#define CF_gray69    RGB2FLOAT(176,   176,   176)
#define CF_gray70    RGB2FLOAT(179,   179,   179)
#define CF_gray71    RGB2FLOAT(181,   181,   181)
#define CF_gray72    RGB2FLOAT(184,   184,   184)
#define CF_gray73    RGB2FLOAT(186,   186,   186)
#define CF_gray74    RGB2FLOAT(189,   189,   189)
#define CF_gray    RGB2FLOAT(190,   190,   190)
#define CF_gray75    RGB2FLOAT(191,   191,   191)
#define CF_gray76    RGB2FLOAT(194,   194,   194)
#define CF_gray77    RGB2FLOAT(196,   196,   196)
#define CF_gray78    RGB2FLOAT(199,   199,   199)
#define CF_gray79    RGB2FLOAT(201,   201,   201)
#define CF_gray80    RGB2FLOAT(204,   204,   204)
#define CF_gray81    RGB2FLOAT(207,   207,   207)
#define CF_gray82    RGB2FLOAT(209,   209,   209)
#define CF_lightgray    RGB2FLOAT(211,   211,   211)
#define CF_gray83    RGB2FLOAT(212,   212,   212)
#define CF_gray84    RGB2FLOAT(214,   214,   214)
#define CF_gray85    RGB2FLOAT(217,   217,   217)
#define CF_gray86    RGB2FLOAT(219,   219,   219)
#define CF_gray87    RGB2FLOAT(222,   222,   222)
#define CF_gray88    RGB2FLOAT(224,   224,   224)
#define CF_gray89    RGB2FLOAT(227,   227,   227)
#define CF_gray90    RGB2FLOAT(229,   229,   229)
#define CF_gray91    RGB2FLOAT(232,   232,   232)
#define CF_gray92    RGB2FLOAT(235,   235,   235)
#define CF_gray93    RGB2FLOAT(237,   237,   237)
#define CF_gray94    RGB2FLOAT(240,   240,   240)
#define CF_gray95    RGB2FLOAT(242,   242,   242)
#define CF_gray96    RGB2FLOAT(245,   245,   245)
#define CF_gray97    RGB2FLOAT(247,   247,   247)
#define CF_gray98    RGB2FLOAT(250,   250,   250)
#define CF_gray99    RGB2FLOAT(252,   252,   252)
#define CF_gray100    RGB2FLOAT(255,   255,   255)
#define CF_white    RGB2FLOAT(255,   255,   255)
#define CF_grey0    RGB2FLOAT(0,   0,   0)
#define CF_grey1    RGB2FLOAT(3,   3,   3)
#define CF_grey2    RGB2FLOAT(5,   5,   5)
#define CF_grey3    RGB2FLOAT(8,   8,   8)
#define CF_grey4    RGB2FLOAT(10,   10,   10)
#define CF_grey5    RGB2FLOAT(13,   13,   13)
#define CF_grey6    RGB2FLOAT(15,   15,   15)
#define CF_grey7    RGB2FLOAT(18,   18,   18)
#define CF_grey8    RGB2FLOAT(20,   20,   20)
#define CF_grey9    RGB2FLOAT(23,   23,   23)
#define CF_grey10    RGB2FLOAT(26,   26,   26)
#define CF_grey11    RGB2FLOAT(28,   28,   28)
#define CF_grey12    RGB2FLOAT(31,   31,   31)
#define CF_grey13    RGB2FLOAT(33,   33,   33)
#define CF_grey14    RGB2FLOAT(36,   36,   36)
#define CF_grey15    RGB2FLOAT(38,   38,   38)
#define CF_grey16    RGB2FLOAT(41,   41,   41)
#define CF_grey17    RGB2FLOAT(43,   43,   43)
#define CF_grey18    RGB2FLOAT(46,   46,   46)
#define CF_grey19    RGB2FLOAT(48,   48,   48)
#define CF_grey20    RGB2FLOAT(51,   51,   51)
#define CF_grey21    RGB2FLOAT(54,   54,   54)
#define CF_grey22    RGB2FLOAT(56,   56,   56)
#define CF_grey23    RGB2FLOAT(59,   59,   59)
#define CF_grey24    RGB2FLOAT(61,   61,   61)
#define CF_grey25    RGB2FLOAT(64,   64,   64)
#define CF_grey26    RGB2FLOAT(66,   66,   66)
#define CF_grey27    RGB2FLOAT(69,   69,   69)
#define CF_grey28    RGB2FLOAT(71,   71,   71)
#define CF_grey29    RGB2FLOAT(74,   74,   74)
#define CF_grey30    RGB2FLOAT(77,   77,   77)
#define CF_grey31    RGB2FLOAT(79,   79,   79)
#define CF_grey32    RGB2FLOAT(82,   82,   82)
#define CF_grey33    RGB2FLOAT(84,   84,   84)
#define CF_grey34    RGB2FLOAT(87,   87,   87)
#define CF_grey35    RGB2FLOAT(89,   89,   89)
#define CF_grey36    RGB2FLOAT(92,   92,   92)
#define CF_grey37    RGB2FLOAT(94,   94,   94)
#define CF_grey38    RGB2FLOAT(97,   97,   97)
#define CF_grey39    RGB2FLOAT(99,   99,   99)
#define CF_grey40    RGB2FLOAT(102,   102,   102)
#define CF_grey41    RGB2FLOAT(105,   105,   105)
#define CF_dimgrey    RGB2FLOAT(105,   105,   105)
#define CF_grey42    RGB2FLOAT(107,   107,   107)
#define CF_grey43    RGB2FLOAT(110,   110,   110)
#define CF_grey44    RGB2FLOAT(112,   112,   112)
#define CF_grey45    RGB2FLOAT(115,   115,   115)
#define CF_grey46    RGB2FLOAT(117,   117,   117)
#define CF_grey47    RGB2FLOAT(120,   120,   120)
#define CF_grey48    RGB2FLOAT(122,   122,   122)
#define CF_grey49    RGB2FLOAT(125,   125,   125)
#define CF_grey50    RGB2FLOAT(127,   127,   127)
#define CF_grey51    RGB2FLOAT(130,   130,   130)
#define CF_grey52    RGB2FLOAT(133,   133,   133)
#define CF_grey53    RGB2FLOAT(135,   135,   135)
#define CF_grey54    RGB2FLOAT(138,   138,   138)
#define CF_grey55    RGB2FLOAT(140,   140,   140)
#define CF_grey56    RGB2FLOAT(143,   143,   143)
#define CF_grey57    RGB2FLOAT(145,   145,   145)
#define CF_grey58    RGB2FLOAT(148,   148,   148)
#define CF_grey59    RGB2FLOAT(150,   150,   150)
#define CF_grey60    RGB2FLOAT(153,   153,   153)
#define CF_grey61    RGB2FLOAT(156,   156,   156)
#define CF_grey62    RGB2FLOAT(158,   158,   158)
#define CF_grey63    RGB2FLOAT(161,   161,   161)
#define CF_grey64    RGB2FLOAT(163,   163,   163)
#define CF_grey65    RGB2FLOAT(166,   166,   166)
#define CF_darkgrey    RGB2FLOAT(169,   169,   169)
#define CF_grey66    RGB2FLOAT(168,   168,   168)
#define CF_grey67    RGB2FLOAT(171,   171,   171)
#define CF_grey68    RGB2FLOAT(173,   173,   173)
#define CF_grey69    RGB2FLOAT(176,   176,   176)
#define CF_grey70    RGB2FLOAT(179,   179,   179)
#define CF_grey71    RGB2FLOAT(181,   181,   181)
#define CF_grey72    RGB2FLOAT(184,   184,   184)
#define CF_grey73    RGB2FLOAT(186,   186,   186)
#define CF_grey74    RGB2FLOAT(189,   189,   189)
#define CF_grey    RGB2FLOAT(190,   190,   190)
#define CF_grey75    RGB2FLOAT(191,   191,   191)
#define CF_grey76    RGB2FLOAT(194,   194,   194)
#define CF_grey77    RGB2FLOAT(196,   196,   196)
#define CF_grey78    RGB2FLOAT(199,   199,   199)
#define CF_grey79    RGB2FLOAT(201,   201,   201)
#define CF_grey80    RGB2FLOAT(204,   204,   204)
#define CF_grey81    RGB2FLOAT(207,   207,   207)
#define CF_grey82    RGB2FLOAT(209,   209,   209)
#define CF_lightgrey    RGB2FLOAT(211,   211,   211)
#define CF_grey83    RGB2FLOAT(212,   212,   212)
#define CF_grey84    RGB2FLOAT(214,   214,   214)
#define CF_grey85    RGB2FLOAT(217,   217,   217)
#define CF_grey86    RGB2FLOAT(219,   219,   219)
#define CF_grey87    RGB2FLOAT(222,   222,   222)
#define CF_grey88    RGB2FLOAT(224,   224,   224)
#define CF_grey89    RGB2FLOAT(227,   227,   227)
#define CF_grey90    RGB2FLOAT(229,   229,   229)
#define CF_grey91    RGB2FLOAT(232,   232,   232)
#define CF_grey92    RGB2FLOAT(235,   235,   235)
#define CF_grey93    RGB2FLOAT(237,   237,   237)
#define CF_grey94    RGB2FLOAT(240,   240,   240)
#define CF_grey95    RGB2FLOAT(242,   242,   242)
#define CF_grey96    RGB2FLOAT(245,   245,   245)
#define CF_grey97    RGB2FLOAT(247,   247,   247)
#define CF_grey98    RGB2FLOAT(250,   250,   250)
#define CF_grey99    RGB2FLOAT(252,   252,   252)
#define CF_grey100    RGB2FLOAT(255,   255,   255)
#define CF_rainbow0    RGB2FLOAT(255,   0,   0)
#define CF_rainbow1    RGB2FLOAT(255,   165,   0)
#define CF_rainbow2    RGB2FLOAT(240,   235,   0)
#define CF_rainbow3    RGB2FLOAT(0,   128,   0)
#define CF_rainbow4    RGB2FLOAT(0,   0,   255)
#define CF_rainbow5    RGB2FLOAT(75,   0,   130)
#define CF_rainbow6    RGB2FLOAT(238,   130,   238)
#define CF_rainbow7    RGB2FLOAT(0,   0,   0)


