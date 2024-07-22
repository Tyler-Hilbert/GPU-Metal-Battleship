#define NS_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION

#include <iostream>
#include <vector>
#include <Metal/Metal.hpp>
#include <Foundation/Foundation.hpp>




struct SharedData {
    std::atomic_uint flag;
};

int main() {
    // Define the Metal kernel source
    const char* kernelSrc = R"(
        #include <metal_stdlib>
        using namespace metal;

        struct SharedData {
            atomic_uint flag;
        };

        kernel void check_gameover_compute_function(
                                                    constant int  *shipsArr   [[ buffer(0) ]],
                                                    constant int  *attacksArr [[ buffer(1) ]],
                                                    device SharedData* stillAlive [[ buffer(2) ]],
                                                    uint index [[ thread_position_in_grid ]] ) {
            if (attacksArr[index] == 0 && shipsArr[index] != 0) {
                atomic_store_explicit(&stillAlive->flag, 1, memory_order_relaxed);
            }
        }
    )";

    // Create a Metal device
    MTL::Device* device = MTL::CreateSystemDefaultDevice();

    // Create a Metal library from the kernel source
    NS::Error* error = nullptr;
    MTL::Library* library = device->newLibrary(NS::String::string(kernelSrc, NS::UTF8StringEncoding), nullptr, &error);
    if (!library) {
        std::cerr << "Failed to create library: " << error->localizedDescription()->utf8String() << std::endl;
        return -1;
    }

    // Load the kernel function
    MTL::Function* kernelFunction = library->newFunction(NS::String::string("check_gameover_compute_function", NS::UTF8StringEncoding));
    if (!kernelFunction) {
        std::cerr << "Failed to load kernel function" << std::endl;
        return -1;
    }

    // Create a compute pipeline state
    MTL::ComputePipelineState* pipelineState = device->newComputePipelineState(kernelFunction, &error);
    if (!pipelineState) {
        std::cerr << "Failed to create compute pipeline state: " << error->localizedDescription()->utf8String() << std::endl;
        return -1;
    }

    // Create a command queue
    MTL::CommandQueue* commandQueue = device->newCommandQueue();

    
    
    
    
    
    
    
    std::vector<int> ships = {
        0, 1, 0,
        2, 1, 0,
        0, 1, 0
    };
    std::vector<int> attacks = {
        0, 0, 0,
        0, 0, 0,
        0, 0, 0
    };
    printf ("Battleship\n");
    
    // Prepare the shared data
    SharedData sharedData;
    sharedData.flag = 1;
    
    
    
    MTL::Buffer* shipsBuffer;
    MTL::Buffer* attacksBuffer;
    MTL::Buffer* sharedDataBuffer;
    
    
    while (sharedData.flag) {
        
        sharedData.flag = 0;
        
        
        printf("Location: ");
        for (int i = 0; i < attacks.size(); i++) {
            printf ("%d ", i);
        }
        printf ("\n");
        printf ("Attacks:  ");
        for (auto attack : attacks) {
            printf ("%d ", attack);
        }
        printf ("\n");
        
        printf ("enter attack location ");
        int attack;
        scanf("%d", &attack);
        if (
            attack < 0 or
            attack > ships.size() or
            attacks.at(attack) != 0
            ) {
                printf ("Not valid\n");
                continue;
            } else {
                if (ships.at(attack) > 0) {
                    ships.at(attack) *= -1;
                    printf ("HIT!\n");
                    attacks.at(attack) = 9;
                } else {
                    attacks.at(attack) = 1;
                }
                
                // Create buffers for shipsArr and attacksArr
                shipsBuffer = device->newBuffer(ships.data(), ships.size() * sizeof(int), MTL::ResourceStorageModeShared);
                attacksBuffer = device->newBuffer(attacks.data(), attacks.size() * sizeof(int), MTL::ResourceStorageModeShared);
                
                // Create a buffer for the shared data
                sharedDataBuffer = device->newBuffer(&sharedData, sizeof(SharedData), MTL::ResourceStorageModeShared);
                
                // Create a command buffer and encoder
                MTL::CommandBuffer* commandBuffer = commandQueue->commandBuffer();
                MTL::ComputeCommandEncoder* computeEncoder = commandBuffer->computeCommandEncoder();
                
                // Set the pipeline state and buffers
                computeEncoder->setComputePipelineState(pipelineState);
                computeEncoder->setBuffer(shipsBuffer, 0, 0);
                computeEncoder->setBuffer(attacksBuffer, 0, 1);
                computeEncoder->setBuffer(sharedDataBuffer, 0, 2);
                
                // Define the grid and thread group sizes
                MTL::Size gridSize = MTL::Size(ships.size(), 1, 1);
                MTL::Size threadGroupSize = MTL::Size(pipelineState->threadExecutionWidth(), 1, 1);
                
                // Dispatch the threads
                computeEncoder->dispatchThreads(gridSize, threadGroupSize);
                computeEncoder->endEncoding();
                
                // Commit the command buffer and wait for completion
                commandBuffer->commit();
                commandBuffer->waitUntilCompleted();
                
                // Read the flag value
                SharedData* resultPointer = reinterpret_cast<SharedData*>(sharedDataBuffer->contents());
                unsigned int flagValue = resultPointer->flag.load();
                sharedData.flag = flagValue;
            }
    }

    
    
    
    
    
    
    
    
    // Clean up
    pipelineState->release();
    kernelFunction->release();
    library->release();
    commandQueue->release();
    device->release();
    shipsBuffer->release();
    attacksBuffer->release();
    sharedDataBuffer->release();

    return 0;
}


// TODOs
// Do you need a buffer for each?
// 15:15 he creates a buffer for each.
// Is there a boolean option instead of int?
// How to put in for loop?
// Can I just use device bool since I'm not worried about a race condition?
