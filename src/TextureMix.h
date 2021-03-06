//
//  TextureMix.h
//  WAVE_POV
//
//  Created by Jonas Fehr on 23/01/2017.
//
//

#ifndef TextureMix_h
#define TextureMix_h

#define STRINGIFY(x) #x
#include "TextureMixShader.h"

enum BlendModes_Mixer : int {
    BLEND_ADD = 1,
    BLEND_MULTIPLY = 2,
    BLEND_LIGHTEN = 3,
    BLEND_DARKEN = 4,
    BLEND_SUBTRACT = 5,
    BLEND_SCREEN = 6,
    BLEND_AVERAGE = 7,
    BLEND_SOFT_LIGHT = 8,
    BLEND_OVERLAY = 9,
    BLEND_ONTOP = 10
};

class TextureGroup{
public:
    ofParameterGroup parameters;
    ofParameter<float> hue;
    ofParameter<float> saturation;
    ofParameter<float> brightness;
    ofParameter<float> contrast;
    ofParameter<float> opacity;
    ofParameter<int> blendMode;
    
    ofFbo *fbo;
    
    TextureGroup(string name, int blendMode, ofFbo *fbo){
        addParameters(name, blendMode);
        this->fbo = fbo;
    }
    
    void addParameters(string name, int initialBlendMode){
        parameters.setName(name);
        parameters.add(hue.set("hue", ofRandom(1.), 0., 1.));
        parameters.add(saturation.set("saturation", 1., 0., 1.));
        parameters.add(brightness.set("brightness", 1., 0., 1.));
        parameters.add(contrast.set("contrast", .5, 0., 1.));
        parameters.add(opacity.set("opacity", 1., 0., 1.));
        parameters.add(blendMode.set("blendMode", initialBlendMode, 1, 10));
    }

};

class TextureMix{
public:
    void draw(int x, int y, int w, int h){
        shader.begin();
        {
            
            shader.setUniform2f("iResolution", w, h);
            shader.setUniform1f("iGlobalTime", ofGetElapsedTimef()); //tempo p nr 1
            
            for(int i = 0; i < texGroups.size(); i++){
                shader.setUniformTexture("tex"+ofToString(i), texGroups[i].fbo->getTexture(), i);
                shader.setUniform1f("u_contrast_"+ofToString(i), texGroups[i].contrast);
                shader.setUniform1f("u_H_"+ofToString(i), texGroups[i].hue);
                shader.setUniform1f("u_S_"+ofToString(i), texGroups[i].saturation);
                shader.setUniform1f("u_B_"+ofToString(i), texGroups[i].brightness);
                shader.setUniform1f("u_opacity_"+ofToString(i), texGroups[i].opacity);
                shader.setUniform1i("u_blendMode_"+ofToString(i), texGroups[i].blendMode);
                shader.setUniform2f("resolution_"+ofToString(i), texGroups[i].fbo->getWidth(), texGroups[i].fbo->getHeight());
                
            }
            
            ofSetColor(255,255,255);
            ofFill();
            ofDrawRectangle(x, x, w, h);
        }
        shader.end();
        
    }
    
    void addFboChannel(ofFbo * fbo, string name, int blendMode){
        TextureGroup texGroup = TextureGroup(name, blendMode, fbo);
        texGroups.push_back( texGroup );
        setup();
    }
    
    ofParameterGroup* getPointerToParameterGroup(){ return &parameterGroup; }
    
    vector<ofParameterGroup*> getVectorOfParameterSubgroups(){
        
        vector<ofParameterGroup*> paramSubGroups;
        
        for(int i = 0; i < texGroups.size(); i++){
            paramSubGroups.push_back(&texGroups[i].parameters);
        }
        return paramSubGroups;
    }

private:
    vector <TextureGroup> texGroups;
    ofShader shader;
    
    ofParameterGroup parameterGroup;
    
    void setup(){
        
        //ParameterGroup
        parameterGroup.clear();
        parameterGroup.setName("Mixer");
        for(int i = 0; i < texGroups.size(); i++){
            parameterGroup.add(texGroups[i].parameters);
        }
        
        // GENERATE THE SHADER
        stringstream shaderScript;
        shaderScript << "#version 120" << endl;
        shaderScript << uniformsHeader;
        for(int i = 0; i < texGroups.size(); i++){
            shaderScript << replaceAll(uniforms, "$0", ofToString(i));
        }
        shaderScript << functions;
        shaderScript << mainHeader;
        for(int i = 0; i < texGroups.size(); i++){
            shaderScript << replaceAll(channel, "$0", ofToString(i));
        }
        shaderScript << output;
        
        shader.setupShaderFromSource(GL_FRAGMENT_SHADER, shaderScript.str());
        shader.linkProgram();
    }
    
    // function from http://stackoverflow.com/questions/3418231/replace-part-of-a-string-with-another-string
    string replaceAll(std::string str, const std::string& from, const std::string& to) {
        if(from.empty())
            return "";
        size_t start_pos = 0;
        while((start_pos = str.find(from, start_pos)) != std::string::npos) {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
        }
        return str;
    }
};


#endif /* TextureMix_h */
