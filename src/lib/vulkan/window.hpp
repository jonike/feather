/***********************************************************************
 *
 * Filename: window.hpp
 *
 * Description: base vulkan window.
 *
 * Copyright (C) 2016 Richard Layman, rlayman2000@yahoo.com 
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 ***********************************************************************/

#ifndef VULKAN_HPP
#define VULKAN_HPP

#include "deps.hpp"
#include "vulkan_deps.hpp"
#include "swapchain.hpp"
#include "textureloader.hpp"
#include "mesh.hpp"

#define VERTEX_BUFFER_BIND_ID 0
#define ENABLE_VALIDATION false
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

namespace feather
{

    namespace vulkan
    {

        class Window
        {

            public:
                Window(std::string _title="", unsigned int _width=1280, unsigned int _height=720, float _zoom=-2.0, bool _validation=false);
                ~Window();
                std::vector<unsigned int> cameras();
                unsigned int current_camera();
                void set_camera(unsigned int uid);

            private:
                std::string m_title;
                unsigned int m_width;
                unsigned int m_height;
                float m_zoom;
                bool m_validation;
                bool m_prepared;
                bool m_quit;
                feather::vulkan::tools::VulkanTextureLoader* m_pTextureLoader;
                glm::vec3 m_rotation;
                glm::vec2 m_mousePos;
                // Active frame buffer index
                uint32_t m_currentBuffer;
                float m_frameTimer;
	
                xcb_connection_t* m_pConnection;
                xcb_screen_t* m_pScreen;
                xcb_window_t m_window;
                xcb_intern_atom_reply_t* m_pAtomDeleteWindow;
                void handleEvent(const xcb_generic_event_t *event);

                struct {
                    bool left = false;
                    bool right = false;
                } m_mouseButtons;

                void initConnection();
                void initVulkan(bool validation);
                xcb_window_t setupWindow();
                void initSwapChain();
                void prepare();
                void renderLoop();

                // Prepare - setup vulkan 
                void createCommandPool();
                void createSetupCommandBuffer();
                void setupSwapChain();
                void createCommandBuffers();
                void setupDepthStencil();
                void setupRenderPass();
                void createPipelineCache();
                void setupFrameBuffer();
                void flushSetupCommandBuffer();

                // MOVED TO NODE
                /*
                // Prepare - setup data
                void prepareSemaphore();
                void prepareVertices();
                void prepareUniformBuffers();
                void setupDescriptorSetLayout();
                void preparePipelines();
                void setupDescriptorPool();
                void setupDescriptorSet();
                void buildCommandBuffers();

                void updateUniformBuffers();
                */


                void keyPressed(uint32_t keyCode);
                // MOVED TO NODE
                //void viewChanged();
                void render();
                void draw();

                // VULKAN MEMBERS
                VkPhysicalDevice m_physicalDevice;
                VkPhysicalDeviceMemoryProperties m_deviceMemoryProperties;
                VkDevice m_device;
                VkQueue m_queue;
                VkInstance m_instance;
                // Contains command buffers and semaphores to be presented to the queue
                VkSubmitInfo m_submitInfo;
                // Pipeline stage flags for the submit info structure
                VkPipelineStageFlags submitPipelineStages = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
                // Depth format is selected during Vulkan initialization
                VkFormat m_depthFormat;
                VulkanSwapChain m_swapChain;
                VkCommandPool m_commandPool;
		VkCommandBuffer m_setupCommandBuffer;
                std::vector<VkCommandBuffer> m_drawCommandBuffers;
                // Command buffer for submitting a post present image barrier
                VkCommandBuffer m_postPresentCommandBuffer;
                // Command buffer for submitting a pre present image barrier
                VkCommandBuffer m_prePresentCommandBuffer;
                // Color buffer format
                VkFormat m_colorFormat;
                // Global render pass for framebuffer writes
                VkRenderPass m_renderPass;
                // Pipeline cache object
                VkPipelineCache m_pipelineCache;
                VkPipelineLayout m_pipelineLayout;
                VkDescriptorSet m_descriptorSet;
                VkDescriptorSetLayout m_descriptorSetLayout;
                VkDescriptorPool m_descriptorPool;
                // List of available frame buffers (same as number of swap chain images)
                std::vector<VkFramebuffer> m_aFrameBuffers;
                // List of shader modules created (stored for cleanup)
                std::vector<VkShaderModule> m_shaderModules;
                VkClearColorValue m_defaultClearColor;


                // Synchronization semaphores
                struct {
                    VkSemaphore presentComplete;
                    VkSemaphore renderComplete;
                } m_semaphores;

                struct 
                {
                    VkImage image;
                    VkDeviceMemory mem;
                    VkImageView view;
                } m_depthStencil;

                VkResult createInstance(bool enabled);
                VkResult createDevice(VkDeviceQueueCreateInfo requestedQueues, bool validation);
                VkBool32 getMemoryType(uint32_t typeBits, VkFlags properties, uint32_t *typeIndex);
                // MOVED TO NODE
                //VkPipelineShaderStageCreateInfo loadShader(const char * fileName, VkShaderStageFlagBits stage);
                
                // MESH TESTING

                struct {
                    VkBuffer buf;
                    VkDeviceMemory mem;
                    VkPipelineVertexInputStateCreateInfo vi;
                    std::vector<VkVertexInputBindingDescription> bindingDescriptions;
                    std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
                } m_vertices;

                struct {
                    int count;
                    VkBuffer buf;
                    VkDeviceMemory mem;
                } m_indices;

                struct {
                    VkBuffer buffer;
                    VkDeviceMemory memory;
                    VkDescriptorBufferInfo descriptor;
                }  m_uniformDataVS;

                struct {
                    glm::mat4 projectionMatrix;
                    glm::mat4 modelMatrix;
                    glm::mat4 viewMatrix;
                } m_uboVS;

                struct {
                    VkPipeline solid;
                } m_pipelines;

                // Feather Methods
                void load_sg();
                std::vector<Node> m_aNodes;
        };

    } // namespace vulkan

} // namespace feather

#endif
