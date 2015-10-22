/******************************************************************************
 * The MIT License (MIT)
 * 
 * Copyright (c) 2015 Baldur Karlsson
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 ******************************************************************************/

#include "../vk_core.h"

#include "driver/shaders/spirv/spirv_common.h"

// Shader functions
bool WrappedVulkan::Serialise_vkCreatePipelineLayout(
		Serialiser*                                 localSerialiser,
		VkDevice                                    device,
		const VkPipelineLayoutCreateInfo*           pCreateInfo,
		VkPipelineLayout*                           pPipelineLayout)
{
	SERIALISE_ELEMENT(ResourceId, devId, GetResID(device));
	SERIALISE_ELEMENT(VkPipelineLayoutCreateInfo, info, *pCreateInfo);
	SERIALISE_ELEMENT(ResourceId, id, GetResID(*pPipelineLayout));

	if(m_State == READING)
	{
		VkPipelineLayout layout = VK_NULL_HANDLE;

		device = GetResourceManager()->GetLiveHandle<VkDevice>(devId);

		VkResult ret = ObjDisp(device)->CreatePipelineLayout(Unwrap(device), &info, &layout);

		if(ret != VK_SUCCESS)
		{
			RDCERR("Failed on resource serialise-creation, VkResult: 0x%08x", ret);
		}
		else
		{
			ResourceId live = GetResourceManager()->WrapResource(Unwrap(device), layout);
			GetResourceManager()->AddLiveResource(id, layout);

			m_CreationInfo.m_PipelineLayout[live].Init(&info);
		}
	}

	return true;
}

VkResult WrappedVulkan::vkCreatePipelineLayout(
		VkDevice                                    device,
		const VkPipelineLayoutCreateInfo*           pCreateInfo,
		VkPipelineLayout*                           pPipelineLayout)
{
	VkDescriptorSetLayout *unwrapped = GetTempArray<VkDescriptorSetLayout>(pCreateInfo->descriptorSetCount);
	for(uint32_t i=0; i < pCreateInfo->descriptorSetCount; i++) unwrapped[i] = Unwrap(pCreateInfo->pSetLayouts[i]);

	VkPipelineLayoutCreateInfo unwrappedInfo = *pCreateInfo;
	unwrappedInfo.pSetLayouts = unwrapped;

	VkResult ret = ObjDisp(device)->CreatePipelineLayout(Unwrap(device), &unwrappedInfo, pPipelineLayout);

	if(ret == VK_SUCCESS)
	{
		ResourceId id = GetResourceManager()->WrapResource(Unwrap(device), *pPipelineLayout);

		if(m_State >= WRITING)
		{
			Chunk *chunk = NULL;

			{
				CACHE_THREAD_SERIALISER();
		
				SCOPED_SERIALISE_CONTEXT(CREATE_PIPE_LAYOUT);
				Serialise_vkCreatePipelineLayout(localSerialiser, device, pCreateInfo, pPipelineLayout);

				chunk = scope.Get();
			}

			VkResourceRecord *record = GetResourceManager()->AddResourceRecord(*pPipelineLayout);
			record->AddChunk(chunk);

			for(uint32_t i=0; i < pCreateInfo->descriptorSetCount; i++)
			{
				VkResourceRecord *layoutrecord = GetRecord(pCreateInfo->pSetLayouts[i]);
				record->AddParent(layoutrecord);
			}
		}
		else
		{
			GetResourceManager()->AddLiveResource(id, *pPipelineLayout);

			m_CreationInfo.m_PipelineLayout[id].Init(pCreateInfo);
		}
	}

	return ret;
}

bool WrappedVulkan::Serialise_vkCreateShaderModule(
		Serialiser*                                 localSerialiser,
		VkDevice                                    device,
		const VkShaderModuleCreateInfo*             pCreateInfo,
		VkShaderModule*                             pShaderModule)
{
	SERIALISE_ELEMENT(ResourceId, devId, GetResID(device));
	SERIALISE_ELEMENT(VkShaderModuleCreateInfo, info, *pCreateInfo);
	SERIALISE_ELEMENT(ResourceId, id, GetResID(*pShaderModule));

	if(m_State == READING)
	{
		device = GetResourceManager()->GetLiveHandle<VkDevice>(devId);
		VkShaderModule sh = VK_NULL_HANDLE;

		VkResult ret = ObjDisp(device)->CreateShaderModule(Unwrap(device), &info, &sh);

		if(ret != VK_SUCCESS)
		{
			RDCERR("Failed on resource serialise-creation, VkResult: 0x%08x", ret);
		}
		else
		{
			ResourceId live = GetResourceManager()->WrapResource(Unwrap(device), sh);
			GetResourceManager()->AddLiveResource(id, sh);

			m_CreationInfo.m_ShaderModule[live].Init(&info);
		}
	}

	return true;
}

VkResult WrappedVulkan::vkCreateShaderModule(
		VkDevice                                    device,
		const VkShaderModuleCreateInfo*             pCreateInfo,
		VkShaderModule*                             pShaderModule)
{
	VkResult ret = ObjDisp(device)->CreateShaderModule(Unwrap(device), pCreateInfo, pShaderModule);

	if(ret == VK_SUCCESS)
	{
		ResourceId id = GetResourceManager()->WrapResource(Unwrap(device), *pShaderModule);
		
		if(m_State >= WRITING)
		{
			Chunk *chunk = NULL;

			{
				CACHE_THREAD_SERIALISER();
		
				SCOPED_SERIALISE_CONTEXT(CREATE_SHADER_MODULE);
				Serialise_vkCreateShaderModule(localSerialiser, device, pCreateInfo, pShaderModule);

				chunk = scope.Get();
			}

			VkResourceRecord *record = GetResourceManager()->AddResourceRecord(*pShaderModule);
			record->AddChunk(chunk);
		}
		else
		{
			GetResourceManager()->AddLiveResource(id, *pShaderModule);
		}
	}

	return ret;
}

bool WrappedVulkan::Serialise_vkCreateShader(
		Serialiser*                                 localSerialiser,
    VkDevice                                    device,
    const VkShaderCreateInfo*                   pCreateInfo,
    VkShader*                                   pShader)
{
	SERIALISE_ELEMENT(ResourceId, devId, GetResID(device));
	SERIALISE_ELEMENT(VkShaderCreateInfo, info, *pCreateInfo);
	SERIALISE_ELEMENT(ResourceId, id, GetResID(*pShader));

	if(m_State == READING)
	{
		device = GetResourceManager()->GetLiveHandle<VkDevice>(devId);
		VkShader sh = VK_NULL_HANDLE;

		VkResult ret = ObjDisp(device)->CreateShader(Unwrap(device), &info, &sh);

		if(ret != VK_SUCCESS)
		{
			RDCERR("Failed on resource serialise-creation, VkResult: 0x%08x", ret);
		}
		else
		{
			ResourceId live = GetResourceManager()->WrapResource(Unwrap(device), sh);
			GetResourceManager()->AddLiveResource(id, sh);

			ResourceId moduleid = GetResourceManager()->GetNonDispWrapper(info.module)->id;
			m_CreationInfo.m_Shader[live].Init(&info, m_CreationInfo.m_ShaderModule[moduleid]);
		}
	}

	return true;
}

VkResult WrappedVulkan::vkCreateShader(
    VkDevice                                    device,
    const VkShaderCreateInfo*                   pCreateInfo,
    VkShader*                                   pShader)
{
	VkShaderCreateInfo unwrappedInfo = *pCreateInfo;
	unwrappedInfo.module = Unwrap(unwrappedInfo.module);
	VkResult ret = ObjDisp(device)->CreateShader(Unwrap(device), &unwrappedInfo, pShader);

	if(ret == VK_SUCCESS)
	{
		ResourceId id = GetResourceManager()->WrapResource(Unwrap(device), *pShader);
		
		if(m_State >= WRITING)
		{
			Chunk *chunk = NULL;

			{
				CACHE_THREAD_SERIALISER();
		
				SCOPED_SERIALISE_CONTEXT(CREATE_SHADER);
				Serialise_vkCreateShader(localSerialiser, device, pCreateInfo, pShader);

				chunk = scope.Get();
			}

			VkResourceRecord *record = GetResourceManager()->AddResourceRecord(*pShader);
			record->AddChunk(chunk);

			VkResourceRecord *modulerecord = GetRecord(pCreateInfo->module);
			record->AddParent(modulerecord);
		}
		else
		{
			GetResourceManager()->AddLiveResource(id, *pShader);
		}
	}

	return ret;
}

// Pipeline functions

bool WrappedVulkan::Serialise_vkCreatePipelineCache(
		Serialiser*                                 localSerialiser,
		VkDevice                                    device,
		const VkPipelineCacheCreateInfo*            pCreateInfo,
		VkPipelineCache*                            pPipelineCache)
{
	SERIALISE_ELEMENT(ResourceId, devId, GetResID(device));
	SERIALISE_ELEMENT(VkPipelineCacheCreateInfo, info, *pCreateInfo);
	SERIALISE_ELEMENT(ResourceId, id, GetResID(*pPipelineCache));

	if(m_State == READING)
	{
		device = GetResourceManager()->GetLiveHandle<VkDevice>(devId);
		VkPipelineCache cache = VK_NULL_HANDLE;

		VkResult ret = ObjDisp(device)->CreatePipelineCache(Unwrap(device), &info, &cache);

		if(ret != VK_SUCCESS)
		{
			RDCERR("Failed on resource serialise-creation, VkResult: 0x%08x", ret);
		}
		else
		{
			ResourceId live = GetResourceManager()->WrapResource(Unwrap(device), cache);
			GetResourceManager()->AddLiveResource(id, cache);
		}
	}

	return true;
}

VkResult WrappedVulkan::vkCreatePipelineCache(
		VkDevice                                    device,
		const VkPipelineCacheCreateInfo*            pCreateInfo,
		VkPipelineCache*                            pPipelineCache)
{
	VkResult ret = ObjDisp(device)->CreatePipelineCache(Unwrap(device), pCreateInfo, pPipelineCache);

	if(ret == VK_SUCCESS)
	{
		ResourceId id = GetResourceManager()->WrapResource(Unwrap(device), *pPipelineCache);
		
		if(m_State >= WRITING)
		{
			Chunk *chunk = NULL;

			{
				CACHE_THREAD_SERIALISER();
		
				SCOPED_SERIALISE_CONTEXT(CREATE_PIPE_CACHE);
				Serialise_vkCreatePipelineCache(localSerialiser, device, pCreateInfo, pPipelineCache);

				chunk = scope.Get();
			}

			VkResourceRecord *record = GetResourceManager()->AddResourceRecord(*pPipelineCache);
			record->AddChunk(chunk);
		}
		else
		{
			GetResourceManager()->AddLiveResource(id, *pPipelineCache);
		}
	}

	return ret;
}

bool WrappedVulkan::Serialise_vkCreateGraphicsPipelines(
		Serialiser*                                 localSerialiser,
		VkDevice                                    device,
		VkPipelineCache                             pipelineCache,
		uint32_t                                    count,
		const VkGraphicsPipelineCreateInfo*         pCreateInfos,
		VkPipeline*                                 pPipelines)
{
	SERIALISE_ELEMENT(ResourceId, devId, GetResID(device));
	SERIALISE_ELEMENT(ResourceId, cacheId, GetResID(pipelineCache));
	SERIALISE_ELEMENT(VkGraphicsPipelineCreateInfo, info, *pCreateInfos);
	SERIALISE_ELEMENT(ResourceId, id, GetResID(*pPipelines));

	if(m_State == READING)
	{
		VkPipeline pipe = VK_NULL_HANDLE;

		device = GetResourceManager()->GetLiveHandle<VkDevice>(devId);
		pipelineCache = GetResourceManager()->GetLiveHandle<VkPipelineCache>(cacheId);

		VkResult ret = ObjDisp(device)->CreateGraphicsPipelines(Unwrap(device), Unwrap(pipelineCache), 1, &info, &pipe);

		if(ret != VK_SUCCESS)
		{
			RDCERR("Failed on resource serialise-creation, VkResult: 0x%08x", ret);
		}
		else
		{
			ResourceId live = GetResourceManager()->WrapResource(Unwrap(device), pipe);
			GetResourceManager()->AddLiveResource(id, pipe);
		
			m_CreationInfo.m_Pipeline[live].Init(&info);
		}
	}

	return true;
}

VkResult WrappedVulkan::vkCreateGraphicsPipelines(
			VkDevice                                    device,
			VkPipelineCache                             pipelineCache,
			uint32_t                                    count,
			const VkGraphicsPipelineCreateInfo*         pCreateInfos,
			VkPipeline*                                 pPipelines)
{
	// conservatively request memory for 5 stages on each pipeline
	// (worst case - can't have compute stage). Avoids needing to count
	byte *unwrapped = GetTempMemory(sizeof(VkGraphicsPipelineCreateInfo)*count + sizeof(VkPipelineShaderStageCreateInfo)*count*5);

	// keep pipelines first in the memory, then the stages
	VkGraphicsPipelineCreateInfo *unwrappedInfos = (VkGraphicsPipelineCreateInfo *)unwrapped;
	VkPipelineShaderStageCreateInfo *nextUnwrappedStages = (VkPipelineShaderStageCreateInfo *)(unwrappedInfos + count);

	for(uint32_t i=0; i < count; i++)
	{
		VkPipelineShaderStageCreateInfo *unwrappedStages = nextUnwrappedStages; nextUnwrappedStages += pCreateInfos[i].stageCount;
		for(uint32_t j=0; j < pCreateInfos[i].stageCount; j++)
		{
			unwrappedStages[j] = pCreateInfos[i].pStages[j];
			unwrappedStages[j].shader = Unwrap(unwrappedStages[j].shader);
		}

		unwrappedInfos[i] = pCreateInfos[i];
		unwrappedInfos[i].pStages = unwrappedStages;
		unwrappedInfos[i].layout = Unwrap(unwrappedInfos[i].layout);
		unwrappedInfos[i].renderPass = Unwrap(unwrappedInfos[i].renderPass);
		unwrappedInfos[i].basePipelineHandle = Unwrap(unwrappedInfos[i].basePipelineHandle);
	}

	VkResult ret = ObjDisp(device)->CreateGraphicsPipelines(Unwrap(device), Unwrap(pipelineCache), count, unwrappedInfos, pPipelines);
	
	if(ret == VK_SUCCESS)
	{
		for(uint32_t i=0; i < count; i++)
		{
			ResourceId id = GetResourceManager()->WrapResource(Unwrap(device), pPipelines[i]);

			if(m_State >= WRITING)
			{
				Chunk *chunk = NULL;

				{
					CACHE_THREAD_SERIALISER();
		
					SCOPED_SERIALISE_CONTEXT(CREATE_GRAPHICS_PIPE);
					Serialise_vkCreateGraphicsPipelines(localSerialiser, device, pipelineCache, 1, &pCreateInfos[i], &pPipelines[i]);

					chunk = scope.Get();
				}

				VkResourceRecord *record = GetResourceManager()->AddResourceRecord(pPipelines[i]);
				record->AddChunk(chunk);

				VkResourceRecord *cacherecord = GetRecord(pipelineCache);
				record->AddParent(cacherecord);

				VkResourceRecord *layoutrecord = GetRecord(pCreateInfos[i].layout);
				record->AddParent(layoutrecord);

				for(uint32_t s=0; s < pCreateInfos[i].stageCount; s++)
				{
					VkResourceRecord *shaderrecord = GetRecord(pCreateInfos[i].pStages[s].shader);
					record->AddParent(shaderrecord);
				}
			}
			else
			{
				GetResourceManager()->AddLiveResource(id, pPipelines[i]);

				m_CreationInfo.m_Pipeline[id].Init(&pCreateInfos[i]);
			}
		}
	}

	return ret;
}

bool WrappedVulkan::Serialise_vkCreateComputePipelines(
		Serialiser*                                 localSerialiser,
		VkDevice                                    device,
		VkPipelineCache                             pipelineCache,
		uint32_t                                    count,
		const VkComputePipelineCreateInfo*          pCreateInfos,
		VkPipeline*                                 pPipelines)
{
	SERIALISE_ELEMENT(ResourceId, devId, GetResID(device));
	SERIALISE_ELEMENT(ResourceId, cacheId, GetResID(pipelineCache));
	SERIALISE_ELEMENT(VkComputePipelineCreateInfo, info, *pCreateInfos);
	SERIALISE_ELEMENT(ResourceId, id, GetResID(*pPipelines));

	if(m_State == READING)
	{
		VkPipeline pipe = VK_NULL_HANDLE;

		device = GetResourceManager()->GetLiveHandle<VkDevice>(devId);
		pipelineCache = GetResourceManager()->GetLiveHandle<VkPipelineCache>(cacheId);

		VkResult ret = ObjDisp(device)->CreateComputePipelines(Unwrap(device), Unwrap(pipelineCache), 1, &info, &pipe);

		if(ret != VK_SUCCESS)
		{
			RDCERR("Failed on resource serialise-creation, VkResult: 0x%08x", ret);
		}
		else
		{
			ResourceId live = GetResourceManager()->WrapResource(Unwrap(device), pipe);
			GetResourceManager()->AddLiveResource(id, pipe);
		
			m_CreationInfo.m_Pipeline[live].Init(&info);
		}
	}

	return true;
}

VkResult WrappedVulkan::vkCreateComputePipelines(
			VkDevice                                    device,
			VkPipelineCache                             pipelineCache,
			uint32_t                                    count,
			const VkComputePipelineCreateInfo*          pCreateInfos,
			VkPipeline*                                 pPipelines)
{
	VkComputePipelineCreateInfo *unwrapped = GetTempArray<VkComputePipelineCreateInfo>(count);

	for(uint32_t i=0; i < count; i++)
	{
		unwrapped[i] = pCreateInfos[i];
		unwrapped[i].stage.shader = Unwrap(unwrapped[i].stage.shader);
		unwrapped[i].layout = Unwrap(unwrapped[i].layout);
		unwrapped[i].basePipelineHandle = Unwrap(unwrapped[i].basePipelineHandle);
	}

	VkResult ret = ObjDisp(device)->CreateComputePipelines(Unwrap(device), Unwrap(pipelineCache), count, unwrapped, pPipelines);
	
	if(ret == VK_SUCCESS)
	{
		for(uint32_t i=0; i < count; i++)
		{
			ResourceId id = GetResourceManager()->WrapResource(Unwrap(device), pPipelines[i]);

			if(m_State >= WRITING)
			{
				Chunk *chunk = NULL;

				{
					CACHE_THREAD_SERIALISER();
		
					SCOPED_SERIALISE_CONTEXT(CREATE_COMPUTE_PIPE);
					Serialise_vkCreateComputePipelines(localSerialiser, device, pipelineCache, 1, &pCreateInfos[i], &pPipelines[i]);

					chunk = scope.Get();
				}

				VkResourceRecord *record = GetResourceManager()->AddResourceRecord(pPipelines[i]);
				record->AddChunk(chunk);

				VkResourceRecord *cacherecord = GetRecord(pipelineCache);
				record->AddParent(cacherecord);

				VkResourceRecord *layoutrecord = GetRecord(pCreateInfos[i].layout);
				record->AddParent(layoutrecord);

				VkResourceRecord *shaderrecord = GetRecord(pCreateInfos[i].stage.shader);
				record->AddParent(shaderrecord);
			}
			else
			{
				GetResourceManager()->AddLiveResource(id, pPipelines[i]);

				m_CreationInfo.m_Pipeline[id].Init(&pCreateInfos[i]);
			}
		}
	}

	return ret;
}
