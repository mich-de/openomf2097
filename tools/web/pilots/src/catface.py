#!/usr/bin/env python3
# Big cat pilot portraits (240x240, transparent around the figure), three-quarter view facing
# right like the original pilots: catface.py <jaguar|tiger> <out.svg>
import math
import random
import sys

kind = sys.argv[1]
out = sys.argv[2]
rnd = random.Random(7 if kind == 'jaguar' else 11)

if kind == 'jaguar':
    fur_dark, fur_mid, fur_light = '#8a4a12', '#d08a2a', '#f2c060'
    cream, cream_shade = '#f6e6c4', '#c9ad84'
    mark = '#1a1008'
    eye = '#c8a020'
    suit, suit_light, suit_trim = '#20283a', '#3a4a68', '#c89a30'
else:
    fur_dark, fur_mid, fur_light = '#9a3a08', '#e0701a', '#ffa848'
    cream, cream_shade = '#fbf6ee', '#cdc2b4'
    mark = '#140a06'
    eye = '#d8a828'
    suit, suit_light, suit_trim = '#2a2a2a', '#4a4a4a', '#d84818'

# head outline: forehead, near ear root, near cheek ruff, chin, muzzle, far cheek, far ear root
HEAD = ('M 66 92 C 62 58 92 34 128 32 C 160 30 186 44 196 70 '
        'C 204 86 206 100 212 112 C 220 124 218 140 208 148 '
        'C 200 158 190 168 176 176 C 160 186 136 188 116 182 '
        'C 96 176 76 170 62 156 C 52 146 46 134 50 122 '
        'C 54 112 60 104 66 92 Z')
MUZZLE = ('M 150 112 C 166 100 196 102 210 114 C 220 124 216 142 204 150 '
          'C 192 160 170 166 152 160 C 140 154 136 128 150 112 Z')
CHIN = 'M 140 158 C 156 172 180 172 196 160 C 190 176 168 184 150 180 C 140 176 136 166 140 158 Z'

def ear(cx, cy, r, tilt):
    a = math.radians(tilt)
    pts = []
    for t in range(0, 181, 15):
        u = math.radians(t)
        x, y = r * math.cos(u), -r * 1.1 * math.sin(u)
        pts.append((cx + x * math.cos(a) - y * math.sin(a), cy + x * math.sin(a) + y * math.cos(a)))
    return 'M ' + ' L '.join('%.1f %.1f' % p for p in pts) + ' Z'

svg = ['<svg xmlns="http://www.w3.org/2000/svg" width="240" height="240" viewBox="0 0 240 240">']
svg.append('''<defs>
<radialGradient id="fur" cx="0.35" cy="0.3" r="0.8">
  <stop offset="0" stop-color="%s"/><stop offset="0.55" stop-color="%s"/><stop offset="1" stop-color="%s"/>
</radialGradient>
<radialGradient id="cream" cx="0.4" cy="0.3" r="0.8">
  <stop offset="0" stop-color="%s"/><stop offset="1" stop-color="%s"/>
</radialGradient>
<linearGradient id="suit" x1="0" y1="0" x2="1" y2="0.4">
  <stop offset="0" stop-color="%s"/><stop offset="1" stop-color="%s"/>
</linearGradient>
<radialGradient id="eye" cx="0.4" cy="0.4" r="0.6">
  <stop offset="0" stop-color="#fff6a0"/><stop offset="1" stop-color="%s"/>
</radialGradient>
<clipPath id="head"><path d="%s"/></clipPath>
<clipPath id="neck"><path d="M 84 150 C 90 196 100 214 104 230 L 180 230 C 182 210 186 190 190 160 Z"/></clipPath>
</defs>''' % (fur_light, fur_mid, fur_dark, cream, cream_shade, suit_light, suit, eye, HEAD))

# shoulders: a pilot's flight suit, collar and trim
svg.append('<path d="M 20 240 C 24 206 52 190 92 186 L 190 186 C 220 192 236 214 238 240 Z" fill="url(#suit)"/>')
svg.append('<path d="M 92 186 L 112 240 L 104 240 L 84 190 Z" fill="%s"/>' % suit_trim)
svg.append('<path d="M 190 186 L 176 240 L 184 240 L 198 190 Z" fill="%s" opacity="0.8"/>' % suit_trim)
# neck fur
svg.append('<path d="M 84 150 C 90 196 100 214 104 230 L 180 230 C 182 210 186 190 190 160 Z" fill="url(#fur)"/>')
svg.append('<path d="M 130 170 C 140 196 150 214 152 230 L 178 230 C 180 210 184 188 186 166 Z" fill="%s" opacity="0.9"/>' % cream)
# ears, the far one behind the head
svg.append('<path d="%s" fill="%s"/>' % (ear(168, 46, 20, 18), fur_dark))
svg.append('<path d="%s" fill="%s"/>' % (ear(168, 48, 12, 18), '#5a3020'))
svg.append('<path d="%s" fill="url(#fur)"/>' % ear(92, 50, 26, -14))
svg.append('<path d="%s" fill="%s"/>' % (ear(93, 54, 16, -14), '#6a3a24'))
svg.append('<path d="%s" fill="%s" opacity="0.8"/>' % (ear(93, 56, 9, -14), cream))
if kind == 'tiger':  # the white spot on the back of a tiger's ear shows on its rim
    svg.append('<path d="%s" fill="%s"/>' % (ear(92, 50, 26, -14), 'none'))
# head
svg.append('<path d="%s" fill="url(#fur)"/>' % HEAD)
svg.append('<g clip-path="url(#head)">')
# cheek ruff, lighter under the eyes
svg.append('<path d="M 56 140 C 70 128 96 124 120 132 C 132 146 132 168 118 184 C 92 182 66 170 56 140 Z" fill="%s" opacity="0.85"/>' % cream)
if kind == 'tiger':
    svg.append('<path d="M 96 70 C 106 64 120 66 126 74 C 118 78 104 78 96 70 Z" fill="%s"/>' % cream)
    svg.append('<path d="M 150 68 C 160 62 172 64 178 72 C 170 76 158 76 150 68 Z" fill="%s"/>' % cream)
# coat markings
if kind == 'jaguar':
    # forehead: small solid spots; cheeks and neck: rosettes (broken rings around a darker center)
    for _ in range(16):
        x, y = rnd.uniform(96, 172), rnd.uniform(40, 80)
        svg.append('<ellipse cx="%.1f" cy="%.1f" rx="%.1f" ry="%.1f" fill="%s"/>' % (x, y, rnd.uniform(2.2, 3.8), rnd.uniform(1.8, 3.2), mark))
    rosettes = []
    while len(rosettes) < 24:
        x, y = rnd.uniform(48, 120), rnd.uniform(96, 186)
        r = rnd.uniform(4.2, 6.2)
        if math.hypot(x - 128, y - 98) < 24 or any(math.hypot(x - a, y - b) < r + c + 3 for a, b, c in rosettes):
            continue
        rosettes.append((x, y, r))
    for x, y, r in rosettes + [(198, 84, 4.5), (190, 68, 4), (176, 54, 4)]:
        svg.append('<circle cx="%.1f" cy="%.1f" r="%.1f" fill="%s"/>' % (x, y, r * 0.6, fur_dark))
        for k in range(4):
            a0 = k * 90 + rnd.uniform(-15, 15)
            a1 = a0 + rnd.uniform(45, 70)
            p0 = (x + r * math.cos(math.radians(a0)), y + r * math.sin(math.radians(a0)))
            p1 = (x + r * math.cos(math.radians(a1)), y + r * math.sin(math.radians(a1)))
            svg.append('<path d="M %.1f %.1f A %.1f %.1f 0 0 1 %.1f %.1f" stroke="%s" stroke-width="%.1f" fill="none" stroke-linecap="round"/>' % (p0 + (r, r) + p1 + (mark, r * 0.42)))
else:
    # stripes: across the forehead, down the cheeks, tapering
    def stripe(x0, y0, x1, y1, w, bend):
        mx, my = (x0 + x1) / 2 + bend[0], (y0 + y1) / 2 + bend[1]
        nx, ny = y1 - y0, x0 - x1
        n = math.hypot(nx, ny) or 1
        nx, ny = nx / n * w, ny / n * w
        return ('<path d="M %.1f %.1f Q %.1f %.1f %.1f %.1f Q %.1f %.1f %.1f %.1f Z" fill="%s"/>'
                % (x0, y0, mx + nx, my + ny, x1, y1, mx - nx * 0.2, my - ny * 0.2, x0, y0, mark))
    for x0, y0, x1, y1, w, b in [
            (112, 36, 118, 58, 3.5, (3, 0)), (126, 34, 130, 60, 4, (2, 0)), (140, 34, 140, 58, 3.5, (-2, 0)),
            (100, 44, 112, 62, 3, (4, 2)), (152, 38, 146, 60, 3, (-3, 1)),
            (118, 68, 136, 70, 2.5, (0, 3)), (106, 80, 124, 82, 2, (0, 2)),
            (54, 104, 92, 112, 4.5, (0, -6)), (52, 122, 94, 126, 5, (0, -6)), (56, 142, 96, 140, 4.5, (2, -6)),
            (66, 160, 100, 152, 4, (2, -4)), (196, 74, 206, 96, 3, (4, 0)), (202, 102, 212, 116, 2.5, (3, 0)),
            (180, 52, 188, 70, 3, (3, 0))]:
        svg.append(stripe(x0, y0, x1, y1, w, b))
# fur: short strands flowing back from the nose, lighter and darker than the coat
for _ in range(520):
    x, y = rnd.uniform(44, 214), rnd.uniform(28, 190)
    a = math.atan2(y - 120, x - 205) + rnd.uniform(-0.3, 0.3)
    l = rnd.uniform(4, 9)
    col = fur_light if rnd.random() < 0.5 else fur_dark
    svg.append('<path d="M %.1f %.1f L %.1f %.1f" stroke="%s" stroke-width="1.1" opacity="0.28" stroke-linecap="round"/>'
               % (x, y, x + l * math.cos(a), y + l * math.sin(a), col))
# brows: heavy, shading the eyes
svg.append('<ellipse cx="128" cy="86" rx="22" ry="9" fill="#000" opacity="0.22"/>')
svg.append('<ellipse cx="182" cy="83" rx="13" ry="7" fill="#000" opacity="0.2"/>')
# shade toward the back of the head and under the jaw
svg.append('<path d="M 40 60 C 60 100 60 150 90 190 L 40 200 Z" fill="#000" opacity="0.18"/>')
svg.append('</g>')
# muzzle and chin
svg.append('<path d="%s" fill="url(#cream)"/>' % MUZZLE)
svg.append('<path d="M 152 150 C 170 162 194 160 210 146 C 204 156 190 166 170 166 C 160 166 150 160 152 150 Z" fill="%s" opacity="0.8"/>' % cream_shade)
svg.append('<path d="%s" fill="%s"/>' % (CHIN, cream_shade))
# whisker pads: rows of dots
for row, y in enumerate((132, 140, 148)):
    for k in range(4):
        svg.append('<circle cx="%.1f" cy="%.1f" r="1.3" fill="%s"/>' % (172 + k * 7 - row * 2, y + k * 0.8, mark))
# whiskers
for y0, dy in ((134, -6), (142, 0), (150, 6)):
    svg.append('<path d="M 196 %d Q 216 %d 232 %d" stroke="#fffaf0" stroke-width="1.1" fill="none" opacity="0.9"/>' % (y0, y0 + dy / 2, y0 + dy))
    svg.append('<path d="M 156 %d Q 140 %d 126 %d" stroke="#fffaf0" stroke-width="1.0" fill="none" opacity="0.7"/>' % (y0, y0 + dy / 2, y0 + dy))
# nose: a broad bridge, the pad, the nostrils
svg.append('<path d="M 160 84 C 176 90 192 100 204 112 L 196 116 C 186 104 172 96 158 92 Z" fill="%s" opacity="0.55"/>' % fur_light)
svg.append('<path d="M 196 112 C 206 108 216 112 216 118 C 214 126 206 130 202 132 C 196 128 190 122 196 112 Z" fill="#8a4a48"/>')
svg.append('<path d="M 200 114 C 206 112 212 114 212 117" stroke="#c88078" stroke-width="2" fill="none"/>')
svg.append('<path d="M 199 128 C 196 124 196 120 199 117" stroke="#3a1a18" stroke-width="2" fill="none"/>')
svg.append('<path d="M 202 132 C 200 140 196 146 188 150 M 202 132 C 206 140 210 144 214 146" stroke="%s" stroke-width="2.2" fill="none" stroke-linecap="round"/>' % mark)
svg.append('<path d="M 176 156 C 186 160 198 158 206 152" stroke="%s" stroke-width="2" fill="none" opacity="0.7"/>' % mark)
# eyes: almond, the near one full, the far one foreshortened; dark rims run down toward the nose
for cx, cy, w, h in ((128, 98, 15, 8.5), (182, 94, 9, 7.5)):
    svg.append('<path d="M %.1f %.1f Q %.1f %.1f %.1f %.1f Q %.1f %.1f %.1f %.1f Z" fill="%s"/>'
               % (cx - w - 3, cy + 1, cx, cy - h - 3, cx + w + 3, cy - 1, cx, cy + h + 3, cx - w - 3, cy + 1, mark))
    svg.append('<ellipse cx="%.1f" cy="%.1f" rx="%.1f" ry="%.1f" fill="url(#eye)"/>' % (cx, cy, w, h))
    svg.append('<ellipse cx="%.1f" cy="%.1f" rx="%.1f" ry="%.1f" fill="#0a0806"/>' % (cx + w * 0.25, cy, w * 0.3, h * 0.8))
    svg.append('<circle cx="%.1f" cy="%.1f" r="%.1f" fill="#fff"/>' % (cx + w * 0.05, cy - h * 0.35, max(1.4, w * 0.13)))
svg.append('<path d="M 142 104 C 150 112 156 120 162 128" stroke="%s" stroke-width="2.5" fill="none" stroke-linecap="round"/>' % mark)
svg.append('<path d="M 110 84 C 120 78 136 78 146 84" stroke="%s" stroke-width="2.4" fill="none" stroke-linecap="round" opacity="0.8"/>' % mark)
svg.append('</svg>')
open(out, 'w').write('\n'.join(svg))
