#version 330 core
out vec4 FragColor;

in vec2 vScreenUV;

uniform mat4 uInvViewProj;  // inverse(P * V)
uniform vec3 uCameraPos;    // Kameraposition in Weltkoords
uniform float uPlaneY = 0.0; // Ebene: y = uPlaneY (Standard: 0)

uniform vec3 uGridColorMinor = vec3(0.55);     // Grau für kleine Linien
uniform vec3 uGridColorMajor = vec3(0.85);     // Hell für Hauptlinien
uniform vec3 uBackground    = vec3(0.0);       // Hintergrund (transparent lassen -> alpha nutzen)
uniform float uMajorStep    = 10.0;            // Jede 10te Line dicker/heller
uniform float uThicknessPx  = 1.0;             // Ziel-Linienstärke in Pixeln
uniform float uFadeStart    = 50.0;            // Entfernung, ab der ausgefadet wird
uniform float uFadeEnd      = 400.0;           // Entfernung, bis zu der vollständig ausgefadet ist

// Achsenfarben (X rot, Z blau, Ursprung stärker)
uniform vec3 uAxisX = vec3(1.0, 0.2, 0.2);
uniform vec3 uAxisZ = vec3(0.2, 0.4, 1.0);
uniform vec3 uAxisOrigin = vec3(1.0);

//
// Hilfsfunktionen
//
vec3 ndcToWorld(vec2 uv, float ndcZ) {
    vec4 ndc = vec4(uv * 2.0 - 1.0, ndcZ, 1.0);
    vec4 w = uInvViewProj * ndc;
    return w.xyz / w.w;
}

bool intersectPlaneY(vec3 ro, vec3 rd, float y, out float t) {
    // ro: ray origin, rd: ray dir (normalisiert), Ebene: y = const
    float denom = rd.y;
    if (abs(denom) < 1e-5) { t = -1.0; return false; }
    t = (y - ro.y) / denom;
    return t > 0.0;
}

// Anti-aliased Grid-Funktion: gibt Linienbedeckung [0..1]
float aaLine(float coord, float thicknessPx, float ddx, float ddy) {
    // Abstand zur nächsten Ganzzahl-Linie
    float a = abs(fract(coord) - 0.5);
    float dist = (0.5 - a) * 2.0; // 0 auf Linie, 1 in Mitte der Zelle
    // Pixel-Skala aus Ableitungen (welt->pixel)
    float w = fwidth(coord); // = |ddx| + |ddy| (Nähe)
    // Ziel-Linienstärke in Weltkoordinaten: thicknessPx * w
    float t = thicknessPx * w;
    return smoothstep(t, 0.0, dist); // mehr Bedeckung nahe der Linie
}

void main() {
    // Ray aus Kamera durch Pixel (verwende nahe & ferne Punkte in Welt, um Richtung zu bilden)
    vec3 worldNear = ndcToWorld(vScreenUV, -1.0);
    vec3 worldFar  = ndcToWorld(vScreenUV,  1.0);
    vec3 ro = uCameraPos;
    vec3 rd = normalize(worldFar - worldNear);

    // Schnitt mit y-Ebene
    float tHit;
    if (!intersectPlaneY(ro, rd, uPlaneY, tHit)) {
        discard; // Blick parallel zur Ebene -> nix zu zeichnen
    }
    vec3 P = ro + rd * tHit; // Treffpunkt im Weltkoordsystem (auf der XZ-Ebene)

    // Entfernung für Fade
    float distCam = length(P - ro);
    float fade = clamp(1.0 - smoothstep(uFadeStart, uFadeEnd, distCam), 0.0, 1.0);
    if (fade <= 0.001) discard;

    // LOD: wähle Zellgröße anhand der Pixelgröße am Treffpunkt
    // Idee: je nach Distanz größere Zellen, damit die Linien stabil bleiben
    // Nutze Ableitungen in Weltkoords für X/Z
    float sx = length(vec2(dFdx(P.x), dFdy(P.x)));
    float sz = length(vec2(dFdx(P.z), dFdy(P.z)));
    float pixelWorld = max(sx, sz) + 1e-6;

    // Basiscell ≈ 1.0, aber skaliere logarithmisch (Zweierpotenzen)
    float rawLevel = max(0.0, floor(log2(pixelWorld * 1.5)));
    float cell = exp2(rawLevel);       // 1,2,4,8,16,…
    float cellMajor = cell * uMajorStep;

    // Minor- und Major-Linienintensität (anti-aliased)
    // Koordinaten im Rastermaß
    vec2 gridCoordMinor = vec2(P.x, P.z) / cell;
    vec2 gridCoordMajor = vec2(P.x, P.z) / cellMajor;

    // Ableitungen für aaLine (nicht strikt nötig, fwidth inside nutzt sie schon)
    float minorX = aaLine(gridCoordMinor.x, uThicknessPx, dFdx(gridCoordMinor.x), dFdy(gridCoordMinor.x));
    float minorZ = aaLine(gridCoordMinor.y, uThicknessPx, dFdx(gridCoordMinor.y), dFdy(gridCoordMinor.y));
    float majorX = aaLine(gridCoordMajor.x, uThicknessPx * 1.75, dFdx(gridCoordMajor.x), dFdy(gridCoordMajor.x));
    float majorZ = aaLine(gridCoordMajor.y, uThicknessPx * 1.75, dFdx(gridCoordMajor.y), dFdy(gridCoordMajor.y));

    float minor = max(minorX, minorZ);
    float major = max(majorX, majorZ);

    // Achsen separat (X-Achse ist Z=0, Z-Achse ist X=0, Ursprung bei X=Z=0)
    float axisX = aaLine(P.z / cell, uThicknessPx * 2.0, dFdx(P.z / cell), dFdy(P.z / cell));
    float axisZ = aaLine(P.x / cell, uThicknessPx * 2.0, dFdx(P.x / cell), dFdy(P.x / cell));
    float axisOrigin = max(aaLine(P.x / (cell * uMajorStep), uThicknessPx * 2.5, 0.0, 0.0),
    aaLine(P.z / (cell * uMajorStep), uThicknessPx * 2.5, 0.0, 0.0));
    axisOrigin *= step(length(P.xz), cell * 0.75); // nur sehr nahe am Ursprung stärker

    // Farben mischen, Achsen priorisieren
    vec3 col = uBackground;

    // Minor/major grid (ohne Achsen)
    float gridMask = clamp(minor * 0.6 + major * 0.9, 0.0, 1.0);
    col = mix(col, mix(uGridColorMinor, uGridColorMajor, major), gridMask);

    // Achsen darüberlegen
    col = mix(col, uAxisX, axisX);
    col = mix(col, uAxisZ, axisZ);
    col = mix(col, uAxisOrigin, axisOrigin);

    // zusätzliche Distanz-Vignette (leichter Kontrastabfall)
    float vignette = smoothstep(0.0, 1.0, fade);
    col *= vignette;

    // Alpha aus Linienbedeckung ableiten + Fade
    float alpha = clamp(max(gridMask, max(axisX, axisZ)) * fade, 0.0, 1.0);

    FragColor = vec4(col, alpha);
}
