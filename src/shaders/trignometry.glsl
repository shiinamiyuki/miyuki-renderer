float AbsCosTheta(vec3 w) { 
	return abs(w.y);
}
float CosTheta(vec3 w) { return w.y; }
float Cos2Theta(vec3 w) { 
	return w.y * w.y; 
}

float Sin2Theta(vec3 w) { 
	return max(0.0, 1 - Cos2Theta(w));
}
float SinTheta(vec3 w) { return sqrt(SinTheta(w)); }
float Tan2Theta(vec3 w) { return Sin2Theta(w) / Cos2Theta(w); }

float TanTheta(vec3 w) { return sqrt(Tan2Theta(w)); }

float CosPhi(vec3 w) {
    float s = SinTheta(w);
    return s == 0.0 ? 1.0 : clamp(w.x / s, -1.0, 1.0);
}
float SinPhi(vec3 w) {
    float s = SinTheta(w);
    return s == 0.0 ? 0.0 : clamp(w.z / s, -1.0, 1.0);
}
float Cos2Phi(vec3 w) {
    float c = CosPhi(w);
    return c * c;
}
float Sin2Phi(vec3 w) {
    float s = SinPhi(w);
    return s * s;
}

float SchlickWeight(float cosTheta) {
    float m = clamp(1.0 - cosTheta, 0.0, 1.0);
    return (m * m) * (m * m) * m;
}
float Schlick(float R0, float cosTheta) { 
	return mix(R0, 1.0, SchlickWeight(cosTheta));
}