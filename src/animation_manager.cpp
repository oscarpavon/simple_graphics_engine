#include "engine.h"
#include "animation_manager.hpp"
#include <iterator>

Animation* AnimationManager::animation_by_name(std::string name){
    for(Animation* anim : this->animations){
        if(anim->name == name){
            return anim;
        }
    }
    std::cout << "Animation: " << name << " not found\n"; 
    return nullptr;
}

void AnimationManager::add_to_queue(SkeletalMesh* skeletal, std::string name, bool loop){
    Animation* anim = animation_by_name(name);   

    if(!anim){
        std::cout << "no playing animation\n";
        return;
    }
    anim->loop = loop;

    AnimationPlay* new_play = new AnimationPlay;
    new_play->animation = anim;
    new_play->skeletal = skeletal;

    animation_to_play.push_back(new_play);
}

/*
Add animation play to animation queue
*/
void AnimationManager::play_animation(SkeletalMesh* skeletal, std::string name, bool loop){
    engine->play_animations = true;

    Animation* anim = animation_by_name(name);
    

    if(!anim){
        std::cout << "no playing animation\n";
        return;
    }

    float time = anim->time;
    
    anim->loop = loop;   
    
    anim->loop = loop;

    play(anim,skeletal);
    
}

void AnimationManager::play(Animation* anim, SkeletalMesh* skeletal){

    float time = anim->time;

    AnimationSampler sampler{};

    for(auto& channel : anim->channels){
        sampler = anim->samplers[channel.sampler_index];

        for( size_t i = 0; i < sampler.inputs.size() - 1 ; i++ ){
            
            if( ( time >= sampler.inputs[i] )  && ( time <= sampler.inputs[i + 1] ) ){
                
                /*  The ratio of those amounts is the fraction of 
                    the interval between timed key frames at which time t appears. 
                */
                float time_mix = (time - sampler.inputs[i] ) / ( sampler.inputs[i+1] - sampler.inputs[i] );

                Node* node = NodeManager::node_from_index(skeletal->mesh,channel.node_index);

                switch (channel.PathType)
                {
                case PATH_TYPE_ROTATION:
                    {
                    glm::quat quat0;
                    quat0.x = sampler.outputs_vec4[i].x;
                    quat0.y = sampler.outputs_vec4[i].y;
                    quat0.z = sampler.outputs_vec4[i].z;
                    quat0.w = sampler.outputs_vec4[i].w;

                    glm::quat quat1;
                    quat1.x = sampler.outputs_vec4[i+1].x;
                    quat1.y = sampler.outputs_vec4[i+1].y;
                    quat1.z = sampler.outputs_vec4[i+1].z;
                    quat1.w = sampler.outputs_vec4[i+1].w;     

                    quat interpolated = normalize( slerp(quat0,quat1,time_mix) );
                    node->Rotation = interpolated;
                    }
                    break;

                case PATH_TYPE_TRANSLATION:
                    vec4 translation = mix(sampler.outputs_vec4[i], sampler.outputs_vec4[i+1], time_mix );
                    node->Translation = vec3(translation);

                    break;
                
                }                
            
            }

        }         
         
    }

    SkeletalManager::update_joints_nodes(skeletal->mesh); 
}

void AnimationManager::play_animations(Engine* engine){
	if(engine->play_animations){
		
        for(auto *anim : animations){
            anim->time += engine->deltaTime;
            if(anim->time >= anim->end){
                if(anim->loop)
                    anim->time = 0;
                else{
                    anim->time = anim->end;
                }
            }
        }
	
	}
    if( !animation_to_play.empty() ){
        std::list<AnimationPlay*> to_remove;
        for(auto *play : animation_to_play){
            if( work_animation_play(play) )
                to_remove.push_back(play);
        }
        for(auto* del_play : to_remove){
            this->animation_to_play.remove(del_play);
            delete del_play;
        }
       
    }
}

bool AnimationManager::work_animation_play(AnimationPlay* play){
  
    Animation* anim = play->animation;
    SkeletalMesh* mesh = play->skeletal;
    anim->time += engine->deltaTime;

    if(anim->time >= anim->end){
            anim->time = 0;    
            return true;
        
    }

    this->play(anim,mesh);
    return false;
}

void AnimationManager::clear_loaders(){

}
AnimationManager::AnimationManager(){
    this->skeletal_loader = new SkeletalLoader();
}
AnimationManager::~AnimationManager(){

}
void AnimationManager::load_animation(SkeletalMesh* skeletal, tinygltf::Model &gltf_model){
    for(auto& anim : gltf_model.animations){
        Animation* new_animation = new Animation;
		new_animation->name = anim.name;

        for(auto& sampler : anim.samplers){
            AnimationSampler new_sampler{};

            {
                //inputs
                const tinygltf::Accessor &input_accessor = gltf_model.accessors[sampler.input];
                const tinygltf::BufferView &input_bufferView = gltf_model.bufferViews[input_accessor.bufferView];
                const tinygltf::Buffer &input_buffer = gltf_model.buffers[input_bufferView.buffer];

                
                const void *dataPtr = &input_buffer.data[input_accessor.byteOffset + input_bufferView.byteOffset];
                const float *buf = static_cast<const float*>(dataPtr);
                for (size_t index = 0; index < input_accessor.count; index++) {
                    new_sampler.inputs.push_back(buf[index]);
                }

                for(float input : new_sampler.inputs){
                    if(input < new_animation->start){
                        new_animation->start = input;
                    }
                    if(input > new_animation->end){
                        new_animation->end = input;
                    }
                }
            }


            //outputs
            const tinygltf::Accessor &accessor = gltf_model.accessors[sampler.output];
            const tinygltf::BufferView &bufferView = gltf_model.bufferViews[accessor.bufferView];
            const tinygltf::Buffer &buffer = gltf_model.buffers[bufferView.buffer];

            const void *dataPtr = &buffer.data[accessor.byteOffset + bufferView.byteOffset];

            switch (accessor.type) {
                case TINYGLTF_TYPE_VEC3: {
                    const glm::vec3 *buf = static_cast<const glm::vec3*>(dataPtr);
                    for (size_t index = 0; index < accessor.count; index++) {
                        new_sampler.outputs_vec4.push_back(glm::vec4(buf[index], 0.0f));
                    }
                    break;
                }
                case TINYGLTF_TYPE_VEC4: {
                    const glm::vec4 *buf = static_cast<const glm::vec4*>(dataPtr);
                    for (size_t index = 0; index < accessor.count; index++) {
                        new_sampler.outputs_vec4.push_back(buf[index]);
                    }
                    break;
                }
                default: {
                    std::cout << "unknown type" << std::endl;
                    break;
                }
            }
             //channels
            for(auto& source : anim.channels){
                AnimationChannel channel{};
                channel.sampler_index = source.sampler;
                channel.node = NodeManager::node_from_index(skeletal,source.target_node);
                channel.node_index = source.target_node;
                
                if(source.target_path == "rotation"){
                    channel.PathType = PATH_TYPE_ROTATION;
                }

                if(source.target_path == "translation"){
                    channel.PathType = PATH_TYPE_TRANSLATION;
                }
                
                new_animation->channels.push_back(channel);
            }

            new_animation->samplers.push_back(new_sampler);

        }        

        skeletal->animations.push_back(new_animation);
		this->animations.push_back(new_animation);
    }
}