#include <cstdlib>

extern const unsigned char lighting_glsl[] = {
    0x2f, 0x2f, 0x20, 0x54, 0x68, 0x69, 0x73, 0x20, 0x66, 0x69, 0x6c, 0x65,
    0x20, 0x69, 0x73, 0x20, 0x70, 0x61, 0x72, 0x74, 0x20, 0x6f, 0x66, 0x20,
    0x53, 0x75, 0x72, 0x66, 0x61, 0x63, 0x65, 0x20, 0x53, 0x70, 0x6c, 0x61,
    0x74, 0x74, 0x69, 0x6e, 0x67, 0x2e, 0x0a, 0x2f, 0x2f, 0x0a, 0x2f, 0x2f,
    0x20, 0x43, 0x6f, 0x70, 0x79, 0x72, 0x69, 0x67, 0x68, 0x74, 0x20, 0x28,
    0x43, 0x29, 0x20, 0x32, 0x30, 0x31, 0x30, 0x2c, 0x20, 0x32, 0x30, 0x31,
    0x35, 0x20, 0x62, 0x79, 0x20, 0x53, 0x65, 0x62, 0x61, 0x73, 0x74, 0x69,
    0x61, 0x6e, 0x20, 0x4c, 0x69, 0x70, 0x70, 0x6f, 0x6e, 0x65, 0x72, 0x2e,
    0x0a, 0x2f, 0x2f, 0x20, 0x0a, 0x2f, 0x2f, 0x20, 0x53, 0x75, 0x72, 0x66,
    0x61, 0x63, 0x65, 0x20, 0x53, 0x70, 0x6c, 0x61, 0x74, 0x74, 0x69, 0x6e,
    0x67, 0x20, 0x69, 0x73, 0x20, 0x66, 0x72, 0x65, 0x65, 0x20, 0x73, 0x6f,
    0x66, 0x74, 0x77, 0x61, 0x72, 0x65, 0x3a, 0x20, 0x79, 0x6f, 0x75, 0x20,
    0x63, 0x61, 0x6e, 0x20, 0x72, 0x65, 0x64, 0x69, 0x73, 0x74, 0x72, 0x69,
    0x62, 0x75, 0x74, 0x65, 0x20, 0x69, 0x74, 0x20, 0x61, 0x6e, 0x64, 0x20,
    0x2f, 0x20, 0x6f, 0x72, 0x20, 0x6d, 0x6f, 0x64, 0x69, 0x66, 0x79, 0x0a,
    0x2f, 0x2f, 0x20, 0x69, 0x74, 0x20, 0x75, 0x6e, 0x64, 0x65, 0x72, 0x20,
    0x74, 0x68, 0x65, 0x20, 0x74, 0x65, 0x72, 0x6d, 0x73, 0x20, 0x6f, 0x66,
    0x20, 0x74, 0x68, 0x65, 0x20, 0x47, 0x4e, 0x55, 0x20, 0x47, 0x65, 0x6e,
    0x65, 0x72, 0x61, 0x6c, 0x20, 0x50, 0x75, 0x62, 0x6c, 0x69, 0x63, 0x20,
    0x4c, 0x69, 0x63, 0x65, 0x6e, 0x73, 0x65, 0x20, 0x61, 0x73, 0x20, 0x70,
    0x75, 0x62, 0x6c, 0x69, 0x73, 0x68, 0x65, 0x64, 0x20, 0x62, 0x79, 0x0a,
    0x2f, 0x2f, 0x20, 0x74, 0x68, 0x65, 0x20, 0x46, 0x72, 0x65, 0x65, 0x20,
    0x53, 0x6f, 0x66, 0x74, 0x77, 0x61, 0x72, 0x65, 0x20, 0x46, 0x6f, 0x75,
    0x6e, 0x64, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x2c, 0x20, 0x65, 0x69, 0x74,
    0x68, 0x65, 0x72, 0x20, 0x76, 0x65, 0x72, 0x73, 0x69, 0x6f, 0x6e, 0x20,
    0x33, 0x20, 0x6f, 0x66, 0x20, 0x74, 0x68, 0x65, 0x20, 0x4c, 0x69, 0x63,
    0x65, 0x6e, 0x73, 0x65, 0x2c, 0x20, 0x6f, 0x72, 0x0a, 0x2f, 0x2f, 0x20,
    0x28, 0x61, 0x74, 0x20, 0x79, 0x6f, 0x75, 0x72, 0x20, 0x6f, 0x70, 0x74,
    0x69, 0x6f, 0x6e, 0x29, 0x20, 0x61, 0x6e, 0x79, 0x20, 0x6c, 0x61, 0x74,
    0x65, 0x72, 0x20, 0x76, 0x65, 0x72, 0x73, 0x69, 0x6f, 0x6e, 0x2e, 0x0a,
    0x2f, 0x2f, 0x20, 0x0a, 0x2f, 0x2f, 0x20, 0x53, 0x75, 0x72, 0x66, 0x61,
    0x63, 0x65, 0x20, 0x53, 0x70, 0x6c, 0x61, 0x74, 0x74, 0x69, 0x6e, 0x67,
    0x20, 0x69, 0x73, 0x20, 0x64, 0x69, 0x73, 0x74, 0x72, 0x69, 0x62, 0x75,
    0x74, 0x65, 0x64, 0x20, 0x69, 0x6e, 0x20, 0x74, 0x68, 0x65, 0x20, 0x68,
    0x6f, 0x70, 0x65, 0x20, 0x74, 0x68, 0x61, 0x74, 0x20, 0x69, 0x74, 0x20,
    0x77, 0x69, 0x6c, 0x6c, 0x20, 0x62, 0x65, 0x20, 0x75, 0x73, 0x65, 0x66,
    0x75, 0x6c, 0x2c, 0x0a, 0x2f, 0x2f, 0x20, 0x62, 0x75, 0x74, 0x20, 0x57,
    0x49, 0x54, 0x48, 0x4f, 0x55, 0x54, 0x20, 0x41, 0x4e, 0x59, 0x20, 0x57,
    0x41, 0x52, 0x52, 0x41, 0x4e, 0x54, 0x59, 0x3b, 0x20, 0x77, 0x69, 0x74,
    0x68, 0x6f, 0x75, 0x74, 0x20, 0x65, 0x76, 0x65, 0x6e, 0x20, 0x74, 0x68,
    0x65, 0x20, 0x69, 0x6d, 0x70, 0x6c, 0x69, 0x65, 0x64, 0x20, 0x77, 0x61,
    0x72, 0x72, 0x61, 0x6e, 0x74, 0x79, 0x20, 0x6f, 0x66, 0x0a, 0x2f, 0x2f,
    0x20, 0x4d, 0x45, 0x52, 0x43, 0x48, 0x41, 0x4e, 0x54, 0x41, 0x42, 0x49,
    0x4c, 0x49, 0x54, 0x59, 0x20, 0x6f, 0x72, 0x20, 0x46, 0x49, 0x54, 0x4e,
    0x45, 0x53, 0x53, 0x20, 0x46, 0x4f, 0x52, 0x20, 0x41, 0x20, 0x50, 0x41,
    0x52, 0x54, 0x49, 0x43, 0x55, 0x4c, 0x41, 0x52, 0x20, 0x50, 0x55, 0x52,
    0x50, 0x4f, 0x53, 0x45, 0x2e, 0x20, 0x53, 0x65, 0x65, 0x20, 0x74, 0x68,
    0x65, 0x0a, 0x2f, 0x2f, 0x20, 0x47, 0x4e, 0x55, 0x20, 0x47, 0x65, 0x6e,
    0x65, 0x72, 0x61, 0x6c, 0x20, 0x50, 0x75, 0x62, 0x6c, 0x69, 0x63, 0x20,
    0x4c, 0x69, 0x63, 0x65, 0x6e, 0x73, 0x65, 0x20, 0x66, 0x6f, 0x72, 0x20,
    0x6d, 0x6f, 0x72, 0x65, 0x20, 0x64, 0x65, 0x74, 0x61, 0x69, 0x6c, 0x73,
    0x2e, 0x0a, 0x2f, 0x2f, 0x20, 0x0a, 0x2f, 0x2f, 0x20, 0x59, 0x6f, 0x75,
    0x20, 0x73, 0x68, 0x6f, 0x75, 0x6c, 0x64, 0x20, 0x68, 0x61, 0x76, 0x65,
    0x20, 0x72, 0x65, 0x63, 0x65, 0x69, 0x76, 0x65, 0x64, 0x20, 0x61, 0x20,
    0x63, 0x6f, 0x70, 0x79, 0x20, 0x6f, 0x66, 0x20, 0x74, 0x68, 0x65, 0x20,
    0x47, 0x4e, 0x55, 0x20, 0x47, 0x65, 0x6e, 0x65, 0x72, 0x61, 0x6c, 0x20,
    0x50, 0x75, 0x62, 0x6c, 0x69, 0x63, 0x20, 0x4c, 0x69, 0x63, 0x65, 0x6e,
    0x73, 0x65, 0x0a, 0x2f, 0x2f, 0x20, 0x61, 0x6c, 0x6f, 0x6e, 0x67, 0x20,
    0x77, 0x69, 0x74, 0x68, 0x20, 0x53, 0x75, 0x72, 0x66, 0x61, 0x63, 0x65,
    0x20, 0x53, 0x70, 0x6c, 0x61, 0x74, 0x74, 0x69, 0x6e, 0x67, 0x2e, 0x20,
    0x49, 0x66, 0x20, 0x6e, 0x6f, 0x74, 0x2c, 0x20, 0x73, 0x65, 0x65, 0x20,
    0x3c, 0x68, 0x74, 0x74, 0x70, 0x3a, 0x2f, 0x2f, 0x77, 0x77, 0x77, 0x2e,
    0x67, 0x6e, 0x75, 0x2e, 0x6f, 0x72, 0x67, 0x2f, 0x6c, 0x69, 0x63, 0x65,
    0x6e, 0x73, 0x65, 0x73, 0x2f, 0x3e, 0x2e, 0x0a, 0x0a, 0x23, 0x76, 0x65,
    0x72, 0x73, 0x69, 0x6f, 0x6e, 0x20, 0x33, 0x33, 0x30, 0x0a, 0x0a, 0x76,
    0x65, 0x63, 0x33, 0x20, 0x6c, 0x69, 0x67, 0x68, 0x74, 0x69, 0x6e, 0x67,
    0x28, 0x76, 0x65, 0x63, 0x33, 0x20, 0x6e, 0x6f, 0x72, 0x6d, 0x61, 0x6c,
    0x5f, 0x65, 0x79, 0x65, 0x2c, 0x20, 0x76, 0x65, 0x63, 0x33, 0x20, 0x76,
    0x5f, 0x65, 0x79, 0x65, 0x2c, 0x20, 0x76, 0x65, 0x63, 0x33, 0x20, 0x63,
    0x6f, 0x6c, 0x6f, 0x72, 0x2c, 0x20, 0x66, 0x6c, 0x6f, 0x61, 0x74, 0x20,
    0x73, 0x68, 0x69, 0x6e, 0x69, 0x6e, 0x65, 0x73, 0x73, 0x29, 0x0a, 0x7b,
    0x0a, 0x20, 0x20, 0x20, 0x20, 0x63, 0x6f, 0x6e, 0x73, 0x74, 0x20, 0x76,
    0x65, 0x63, 0x33, 0x20, 0x6c, 0x69, 0x67, 0x68, 0x74, 0x5f, 0x65, 0x79,
    0x65, 0x20, 0x3d, 0x20, 0x76, 0x65, 0x63, 0x33, 0x28, 0x30, 0x2e, 0x30,
    0x2c, 0x20, 0x30, 0x2e, 0x30, 0x2c, 0x20, 0x31, 0x2e, 0x30, 0x29, 0x3b,
    0x0a, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x66, 0x6c, 0x6f, 0x61, 0x74, 0x20,
    0x64, 0x69, 0x66, 0x20, 0x3d, 0x20, 0x6d, 0x61, 0x78, 0x28, 0x64, 0x6f,
    0x74, 0x28, 0x6c, 0x69, 0x67, 0x68, 0x74, 0x5f, 0x65, 0x79, 0x65, 0x2c,
    0x20, 0x6e, 0x6f, 0x72, 0x6d, 0x61, 0x6c, 0x5f, 0x65, 0x79, 0x65, 0x29,
    0x2c, 0x20, 0x30, 0x2e, 0x30, 0x29, 0x3b, 0x0a, 0x20, 0x20, 0x20, 0x20,
    0x76, 0x65, 0x63, 0x33, 0x20, 0x72, 0x65, 0x66, 0x6c, 0x5f, 0x65, 0x79,
    0x65, 0x20, 0x3d, 0x20, 0x72, 0x65, 0x66, 0x6c, 0x65, 0x63, 0x74, 0x28,
    0x6c, 0x69, 0x67, 0x68, 0x74, 0x5f, 0x65, 0x79, 0x65, 0x2c, 0x20, 0x6e,
    0x6f, 0x72, 0x6d, 0x61, 0x6c, 0x5f, 0x65, 0x79, 0x65, 0x29, 0x3b, 0x0a,
    0x0a, 0x20, 0x20, 0x20, 0x20, 0x76, 0x65, 0x63, 0x33, 0x20, 0x76, 0x69,
    0x65, 0x77, 0x5f, 0x65, 0x79, 0x65, 0x20, 0x3d, 0x20, 0x6e, 0x6f, 0x72,
    0x6d, 0x61, 0x6c, 0x69, 0x7a, 0x65, 0x28, 0x76, 0x5f, 0x65, 0x79, 0x65,
    0x29, 0x3b, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x66, 0x6c, 0x6f, 0x61, 0x74,
    0x20, 0x73, 0x70, 0x65, 0x20, 0x3d, 0x20, 0x70, 0x6f, 0x77, 0x28, 0x63,
    0x6c, 0x61, 0x6d, 0x70, 0x28, 0x64, 0x6f, 0x74, 0x28, 0x72, 0x65, 0x66,
    0x6c, 0x5f, 0x65, 0x79, 0x65, 0x2c, 0x20, 0x76, 0x69, 0x65, 0x77, 0x5f,
    0x65, 0x79, 0x65, 0x29, 0x2c, 0x20, 0x30, 0x2e, 0x30, 0x2c, 0x20, 0x31,
    0x2e, 0x30, 0x29, 0x2c, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x20, 0x20, 0x20, 0x20, 0x20, 0x73, 0x68, 0x69, 0x6e, 0x69, 0x6e, 0x65,
    0x73, 0x73, 0x29, 0x3b, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x66, 0x6c, 0x6f,
    0x61, 0x74, 0x20, 0x72, 0x69, 0x6d, 0x20, 0x3d, 0x20, 0x70, 0x6f, 0x77,
    0x28, 0x31, 0x2e, 0x30, 0x20, 0x2b, 0x20, 0x64, 0x6f, 0x74, 0x28, 0x6e,
    0x6f, 0x72, 0x6d, 0x61, 0x6c, 0x5f, 0x65, 0x79, 0x65, 0x2c, 0x20, 0x76,
    0x69, 0x65, 0x77, 0x5f, 0x65, 0x79, 0x65, 0x29, 0x2c, 0x20, 0x33, 0x2e,
    0x30, 0x29, 0x3b, 0x0a, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x76, 0x65, 0x63,
    0x33, 0x20, 0x72, 0x65, 0x73, 0x20, 0x3d, 0x20, 0x30, 0x2e, 0x31, 0x35,
    0x20, 0x2a, 0x20, 0x63, 0x6f, 0x6c, 0x6f, 0x72, 0x3b, 0x0a, 0x20, 0x20,
    0x20, 0x20, 0x72, 0x65, 0x73, 0x20, 0x2b, 0x3d, 0x20, 0x30, 0x2e, 0x36,
    0x20, 0x2a, 0x20, 0x64, 0x69, 0x66, 0x20, 0x2a, 0x20, 0x63, 0x6f, 0x6c,
    0x6f, 0x72, 0x3b, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x72, 0x65, 0x73, 0x20,
    0x2b, 0x3d, 0x20, 0x30, 0x2e, 0x31, 0x20, 0x2a, 0x20, 0x73, 0x70, 0x65,
    0x20, 0x2a, 0x20, 0x76, 0x65, 0x63, 0x33, 0x28, 0x31, 0x2e, 0x30, 0x29,
    0x3b, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x72, 0x65, 0x73, 0x20, 0x2b, 0x3d,
    0x20, 0x30, 0x2e, 0x31, 0x20, 0x2a, 0x20, 0x72, 0x69, 0x6d, 0x20, 0x2a,
    0x20, 0x76, 0x65, 0x63, 0x33, 0x28, 0x31, 0x2e, 0x30, 0x29, 0x3b, 0x0a,
    0x0a, 0x20, 0x20, 0x20, 0x20, 0x72, 0x65, 0x74, 0x75, 0x72, 0x6e, 0x20,
    0x72, 0x65, 0x73, 0x3b, 0x0a, 0x7d, 0x0a, 0x0 };

const std::size_t lighting_len = sizeof(lighting_glsl);

