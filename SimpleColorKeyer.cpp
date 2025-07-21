// SimpleColorKeyer.cpp - CPU-only version with 6-direction color controls
#include "DDImage/Iop.h"
#include "DDImage/Row.h"
#include "DDImage/Knobs.h"
#include <cmath>
#include <algorithm>

using namespace DD::Image;

struct Color3 {
    float r, g, b;
    Color3() : r(0), g(0), b(0) {}
    Color3(float r_, float g_, float b_) : r(r_), g(g_), b(b_) {}
    float distance_to(const Color3& other) const {
        float dr = r - other.r;
        float dg = g - other.g;
        float db = b - other.b;
        return sqrt(dr*dr + dg*dg + db*db);
    }
};

class SimpleColorKeyerIop : public Iop {
private:
    float key_color_[3];        // RGB color to key
    float variance_;            // Color variance/tolerance
    float range_red_;           // Red direction (-3 to +3)
    float range_magenta_;       // Magenta direction (-3 to +3)
    float range_green_;         // Green direction (-3 to +3)
    float range_yellow_;        // Yellow direction (-3 to +3)
    float range_blue_;          // Blue direction (-3 to +3)
    float range_cyan_;          // Cyan direction (-3 to +3)
    float gain_;               // Alpha gain/contrast
    bool invert_;              // Invert the matte
    int keying_method_;        // 0=distance, 1=chroma, 2=luma weighted, 3=adaptive
    
public:
    SimpleColorKeyerIop(Node* node) : Iop(node) {
        // Default green screen color
        key_color_[0] = 0.0f;  // Red
        key_color_[1] = 1.0f;  // Green  
        key_color_[2] = 0.0f;  // Blue
        
        variance_ = 0.3f;      // 30% tolerance
        range_red_ = 0.0f;     // No red expansion
        range_magenta_ = 0.0f; // No magenta expansion
        range_green_ = 0.0f;   // No green expansion
        range_yellow_ = 0.0f;  // No yellow expansion
        range_blue_ = 0.0f;    // No blue expansion
        range_cyan_ = 0.0f;    // No cyan expansion
        gain_ = 1.0f;          // No gain adjustment
        invert_ = false;       // Normal matte
        keying_method_ = 0;    // Distance-based keying
    }
    
    void _validate(bool for_real) override {
        Iop::_validate(for_real);
        copy_info();
        
        // Ensure alpha channel is always available
        set_out_channels(Mask_RGBA);
        info_.turn_on(Chan_Alpha);
        
        // Force alpha to be part of the output
        if (info_.channels() & Mask_RGB) {
            info_.turn_on(Mask_Alpha);
        }
    }
    
    void _request(int x, int y, int r, int t, ChannelMask channels, int count) override {
        // Always request RGB from input
        ChannelMask input_channels = Mask_RGB;
        
        // If alpha is requested in output, we need to generate it
        if (channels & Mask_Alpha) {
            // We generate alpha from RGB, so just request RGB
        }
        
        input0().request(x, y, r, t, input_channels, count);
    }
    
    void engine(int y, int x, int r, ChannelMask channels, Row& row) override {
        Row input_row(x, r);
        input0().get(y, x, r, Mask_RGB, input_row);
        
        Color3 key_color(key_color_[0], key_color_[1], key_color_[2]);
        
        for (int X = x; X < r; X++) {
            Color3 pixel_color(
                input_row[Chan_Red][X], 
                input_row[Chan_Green][X], 
                input_row[Chan_Blue][X]
            );
            
            float alpha = calculate_alpha(pixel_color, key_color);
            
            // Apply gain
            alpha = alpha * gain_;
            alpha = std::max(0.0f, std::min(1.0f, alpha));
            
            if (invert_) {
                alpha = 1.0f - alpha;
            }
            
            // Output channels - always pass through RGB, alpha goes to alpha channel
            if (channels & Mask_Red)   row.writable(Chan_Red)[X] = input_row[Chan_Red][X];
            if (channels & Mask_Green) row.writable(Chan_Green)[X] = input_row[Chan_Green][X];
            if (channels & Mask_Blue)  row.writable(Chan_Blue)[X] = input_row[Chan_Blue][X];
            
            // ALWAYS output alpha (not conditional)
            row.writable(Chan_Alpha)[X] = alpha;
        }
    }
    
private:
    // Intelligent alpha calculation based on keying method
    float calculate_alpha(const Color3& pixel, const Color3& key) {
        float alpha = 0.0f;
        
        switch (keying_method_) {
            case 0: // Distance-based (default)
                alpha = calculate_distance_alpha(pixel, key);
                break;
            case 1: // Chroma-based (ignore luminance)
                alpha = calculate_chroma_alpha(pixel, key);
                break;
            case 2: // Luma-weighted
                alpha = calculate_luma_weighted_alpha(pixel, key);
                break;
            case 3: // Adaptive (combines multiple methods)
                alpha = calculate_adaptive_alpha(pixel, key);
                break;
        }
        
        return alpha;
    }
    
    float calculate_distance_alpha(const Color3& pixel, const Color3& key) {
        // Calculate how much this pixel matches each of the 6 color directions
        float red_match = pixel.r;                           // Pure red component
        float green_match = pixel.g;                         // Pure green component  
        float blue_match = pixel.b;                          // Pure blue component
        float yellow_match = std::min(pixel.r, pixel.g);     // Yellow = min(R,G)
        float magenta_match = std::min(pixel.r, pixel.b);    // Magenta = min(R,B)
        float cyan_match = std::min(pixel.g, pixel.b);       // Cyan = min(G,B)
        
        // Calculate base distance
        float base_distance = pixel.distance_to(key);
        
        // Start with base tolerance
        float effective_tolerance = variance_;
        
        // Add tolerance expansion based on how much the pixel matches each color direction
        if (range_red_ > 0.0f) {
            effective_tolerance += range_red_ * 0.1f * red_match;
        }
        if (range_green_ > 0.0f) {
            effective_tolerance += range_green_ * 0.1f * green_match;
        }
        if (range_blue_ > 0.0f) {
            effective_tolerance += range_blue_ * 0.1f * blue_match;
        }
        if (range_yellow_ > 0.0f) {
            effective_tolerance += range_yellow_ * 0.1f * yellow_match;
        }
        if (range_magenta_ > 0.0f) {
            effective_tolerance += range_magenta_ * 0.1f * magenta_match;
        }
        if (range_cyan_ > 0.0f) {
            effective_tolerance += range_cyan_ * 0.1f * cyan_match;
        }
        
        // Handle negative values (contract tolerance for those colors)
        if (range_red_ < 0.0f) {
            effective_tolerance += range_red_ * 0.1f * red_match; // This will subtract
        }
        if (range_green_ < 0.0f) {
            effective_tolerance += range_green_ * 0.1f * green_match;
        }
        if (range_blue_ < 0.0f) {
            effective_tolerance += range_blue_ * 0.1f * blue_match;
        }
        if (range_yellow_ < 0.0f) {
            effective_tolerance += range_yellow_ * 0.1f * yellow_match;
        }
        if (range_magenta_ < 0.0f) {
            effective_tolerance += range_magenta_ * 0.1f * magenta_match;
        }
        if (range_cyan_ < 0.0f) {
            effective_tolerance += range_cyan_ * 0.1f * cyan_match;
        }
        
        // Ensure minimum tolerance
        effective_tolerance = std::max(0.001f, effective_tolerance);
        
        // Calculate final alpha
        float normalized_distance = base_distance / effective_tolerance;
        return std::max(0.0f, 1.0f - normalized_distance);
    }
    
    float calculate_chroma_alpha(const Color3& pixel, const Color3& key) {
        // Convert to YUV-like space to ignore luminance
        float pixel_u = pixel.r - pixel.g;
        float pixel_v = pixel.b - pixel.g;
        float key_u = key.r - key.g;
        float key_v = key.b - key.g;
        
        float chroma_distance = sqrtf((pixel_u - key_u) * (pixel_u - key_u) + 
                                     (pixel_v - key_v) * (pixel_v - key_v));
        float normalized_distance = chroma_distance / variance_;
        return std::max(0.0f, 1.0f - normalized_distance);
    }
    
    float calculate_luma_weighted_alpha(const Color3& pixel, const Color3& key) {
        float pixel_luma = 0.299f * pixel.r + 0.587f * pixel.g + 0.114f * pixel.b;
        float key_luma = 0.299f * key.r + 0.587f * key.g + 0.114f * key.b;
        
        float luma_diff = std::abs(pixel_luma - key_luma);
        float luma_weight = 1.0f - std::min(1.0f, luma_diff / 0.5f);
        
        float color_alpha = calculate_distance_alpha(pixel, key);
        return color_alpha * luma_weight;
    }
    
    float calculate_adaptive_alpha(const Color3& pixel, const Color3& key) {
        float distance_alpha = calculate_distance_alpha(pixel, key);
        float chroma_alpha = calculate_chroma_alpha(pixel, key);
        
        // Weight based on how saturated the key color is
        float key_saturation = std::max({key.r, key.g, key.b}) - std::min({key.r, key.g, key.b});
        
        if (key_saturation > 0.5f) {
            // Highly saturated key color - prefer chroma keying
            return 0.3f * distance_alpha + 0.7f * chroma_alpha;
        } else {
            // Less saturated key color - prefer distance keying
            return 0.7f * distance_alpha + 0.3f * chroma_alpha;
        }
    }
    
public:
    void knobs(Knob_Callback f) override {
        Divider(f, "Simple Color Keyer");
        
        Color_knob(f, key_color_, IRange(0, 1), "key_color", "Key Color");
        Tooltip(f, "The base color to key out. Use the color picker to select.");
        
        Float_knob(f, &variance_, IRange(0.001f, 2.0f), "variance", "Tolerance");
        Tooltip(f, "Overall color matching tolerance. Lower values = more precise keying.");
        
        Divider(f, "");

	Newline(f);
        
        static const char* keying_methods[] = {
            "Distance", "Chroma", "Luma Weighted", "Adaptive", nullptr
        };
        Enumeration_knob(f, &keying_method_, keying_methods, "method", "Keying Method");
        Tooltip(f, "Distance: Standard RGB distance (works with color expansion)\n"
                   "Chroma: Ignores brightness changes\n" 
                   "Luma Weighted: Considers brightness similarity\n"
                   "Adaptive: Automatically chooses best method");
        
        Newline(f);
        
        Float_knob(f, &gain_, IRange(0.0f, 5.0f), "gain", "Gain");
        Tooltip(f, "Alpha contrast adjustment. >1.0 increases contrast.");
        
        Bool_knob(f, &invert_, "invert", "Invert");
        Tooltip(f, "Invert the generated matte.");
                
        Newline(f);
        Divider(f, "6-Direction Color Expansion");
        
        BeginGroup(f, "Primary Colors");
        Float_knob(f, &range_red_, IRange(-3.0f, 3.0f), "red_range", "Red");
        Tooltip(f, "Expand keying toward red (+) or away from red (-). Range: -3 to +3");
        Float_knob(f, &range_green_, IRange(-3.0f, 3.0f), "green_range", "Green");
        Tooltip(f, "Expand keying toward green (+) or away from green (-). Range: -3 to +3");
        Float_knob(f, &range_blue_, IRange(-3.0f, 3.0f), "blue_range", "Blue");
        Tooltip(f, "Expand keying toward blue (+) or away from blue (-). Range: -3 to +3");
        EndGroup(f);
        
        BeginGroup(f, "Secondary Colors");
        Float_knob(f, &range_yellow_, IRange(-3.0f, 3.0f), "yellow_range", "Yellow");
        Tooltip(f, "Expand keying toward yellow (+) or away from yellow (-). Range: -3 to +3");
        Float_knob(f, &range_magenta_, IRange(-3.0f, 3.0f), "magenta_range", "Magenta");
        Tooltip(f, "Expand keying toward magenta (+) or away from magenta (-). Range: -3 to +3");
        Float_knob(f, &range_cyan_, IRange(-3.0f, 3.0f), "cyan_range", "Cyan");
        Tooltip(f, "Expand keying toward cyan (+) or away from cyan (-). Range: -3 to +3");
        EndGroup(f);
        
        Divider(f, "");
        
        Text_knob(f, "Simple Color Keyer by Peter Mercell v2.0 2025");
    }
    
    const char* Class() const override { return "SimpleColorKeyer"; }
    const char* node_help() const override {
        return "Simple Color Keyer with 6-Direction Color Control\n\n"
               "An intelligent color keyer with precise 6-direction color expansion control.\n\n"
               "Workflow:\n"
               "1. Pick your base key color with the eyedropper\n"
               "2. Set overall tolerance for the base matching\n"
               "3. Use 6-direction controls (-3 to +3) for precise expansion:\n\n"
               "Primary Colors:\n"
               "• Red: Expand toward/away from red tones\n"
               "• Green: Expand toward/away from green tones\n"
               "• Blue: Expand toward/away from blue tones\n\n"
               "Secondary Colors:\n"
               "• Yellow: Expand toward/away from yellow tones (red+green)\n"
               "• Magenta: Expand toward/away from magenta tones (red+blue)\n"
               "• Cyan: Expand toward/away from cyan tones (green+blue)\n\n"
               "Control Logic:\n"
               "• Positive values (+): Expand keying TOWARD that color\n"
               "• Negative values (-): Contract keying AWAY from that color\n"
               "• Range -3 to +3: Extreme control for challenging footage\n\n"
               "Examples:\n"
               "• Green screen with yellow spill: Green +1.5, Yellow +1.0\n"
               "• Blue screen with cyan cast: Blue +2.0, Cyan +1.0\n"
               "• Red object, avoid orange: Red +1.0, Yellow -0.5\n"
               "• Skin tone, warm variant: Red +0.8, Magenta +0.3, Yellow +0.5\n\n"
               "Keying Methods:\n"
               "• Distance: Works with color expansion (recommended)\n"
               "• Chroma: Ignores brightness changes\n"
               "• Adaptive: Smart automatic selection\n\n"
               "Perfect for precise color isolation with intuitive color wheel control.";
    }
    
    static const Description d;
};

// Plugin registration
static Iop* SimpleColorKeyer_c(Node* node) {
    return new SimpleColorKeyerIop(node);
}

const Iop::Description SimpleColorKeyerIop::d("SimpleColorKeyer", "Keyer/SimpleColorKeyer", SimpleColorKeyer_c);
