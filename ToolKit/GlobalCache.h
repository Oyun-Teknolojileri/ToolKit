#pragma once

#include "Types.h"

namespace ToolKit
{
  // Optimization caches.
  static MeshRawPtrArray g_meshCollector;

  // LightData String cache.
  static StringArray g_lightPosStrCache = {"LightData.pos[0]",
                                           "LightData.pos[1]",
                                           "LightData.pos[2]",
                                           "LightData.pos[3]",
                                           "LightData.pos[4]",
                                           "LightData.pos[5]",
                                           "LightData.pos[6]",
                                           "LightData.pos[7]",
                                           "LightData.pos[8]",
                                           "LightData.pos[9]",
                                           "LightData.pos[10]",
                                           "LightData.pos[11]"};

  static StringArray g_lightDirStrCache = {"LightData.dir[0]",
                                           "LightData.dir[1]",
                                           "LightData.dir[2]",
                                           "LightData.dir[3]",
                                           "LightData.dir[4]",
                                           "LightData.dir[5]",
                                           "LightData.dir[6]",
                                           "LightData.dir[7]",
                                           "LightData.dir[8]",
                                           "LightData.dir[9]",
                                           "LightData.dir[10]",
                                           "LightData.dir[11]"};

  static StringArray g_lightColorStrCache = {"LightData.color[0]",
                                             "LightData.color[1]",
                                             "LightData.color[2]",
                                             "LightData.color[3]",
                                             "LightData.color[4]",
                                             "LightData.color[5]",
                                             "LightData.color[6]",
                                             "LightData.color[7]",
                                             "LightData.color[8]",
                                             "LightData.color[9]",
                                             "LightData.color[10]",
                                             "LightData.color[11]"};

  static StringArray g_lightIntensityStrCache = {"LightData.intensity[0]",
                                                 "LightData.intensity[1]",
                                                 "LightData.intensity[2]",
                                                 "LightData.intensity[3]",
                                                 "LightData.intensity[4]",
                                                 "LightData.intensity[5]",
                                                 "LightData.intensity[6]",
                                                 "LightData.intensity[7]",
                                                 "LightData.intensity[8]",
                                                 "LightData.intensity[9]",
                                                 "LightData.intensity[10]",
                                                 "LightData.intensity[11]"};

  static StringArray g_lightTypeStrCache = {"LightData.type[0]",
                                            "LightData.type[1]",
                                            "LightData.type[2]",
                                            "LightData.type[3]",
                                            "LightData.type[4]",
                                            "LightData.type[5]",
                                            "LightData.type[6]",
                                            "LightData.type[7]",
                                            "LightData.type[8]",
                                            "LightData.type[9]",
                                            "LightData.type[10]",
                                            "LightData.type[11]"};

  static StringArray g_lightRadiusStrCache = {"LightData.radius[0]",
                                              "LightData.radius[1]",
                                              "LightData.radius[2]",
                                              "LightData.radius[3]",
                                              "LightData.radius[4]",
                                              "LightData.radius[5]",
                                              "LightData.radius[6]",
                                              "LightData.radius[7]",
                                              "LightData.radius[8]",
                                              "LightData.radius[9]",
                                              "LightData.radius[10]",
                                              "LightData.radius[11]"};

  static StringArray g_lightOuterAngleStrCache = {"LightData.outAngle[0]",
                                                  "LightData.outAngle[1]",
                                                  "LightData.outAngle[2]",
                                                  "LightData.outAngle[3]",
                                                  "LightData.outAngle[4]",
                                                  "LightData.outAngle[5]",
                                                  "LightData.outAngle[6]",
                                                  "LightData.outAngle[7]",
                                                  "LightData.outAngle[8]",
                                                  "LightData.outAngle[9]",
                                                  "LightData.outAngle[10]",
                                                  "LightData.outAngle[11]"};

  static StringArray g_lightInnerAngleStrCache = {"LightData.innAngle[0]",
                                                  "LightData.innAngle[1]",
                                                  "LightData.innAngle[2]",
                                                  "LightData.innAngle[3]",
                                                  "LightData.innAngle[4]",
                                                  "LightData.innAngle[5]",
                                                  "LightData.innAngle[6]",
                                                  "LightData.innAngle[7]",
                                                  "LightData.innAngle[8]",
                                                  "LightData.innAngle[9]",
                                                  "LightData.innAngle[10]",
                                                  "LightData.innAngle[11]"};

  static StringArray g_lightprojectionViewMatrixStrCache = {
      "LightData.projectionViewMatrix[0]",
      "LightData.projectionViewMatrix[1]",
      "LightData.projectionViewMatrix[2]",
      "LightData.projectionViewMatrix[3]",
      "LightData.projectionViewMatrix[4]",
      "LightData.projectionViewMatrix[5]",
      "LightData.projectionViewMatrix[6]",
      "LightData.projectionViewMatrix[7]",
      "LightData.projectionViewMatrix[8]",
      "LightData.projectionViewMatrix[9]",
      "LightData.projectionViewMatrix[10]",
      "LightData.projectionViewMatrix[11]"};

  static StringArray g_lightCastShadowStrCache = {"LightData.castShadow[0]",
                                                  "LightData.castShadow[1]",
                                                  "LightData.castShadow[2]",
                                                  "LightData.castShadow[3]",
                                                  "LightData.castShadow[4]",
                                                  "LightData.castShadow[5]",
                                                  "LightData.castShadow[6]",
                                                  "LightData.castShadow[7]",
                                                  "LightData.castShadow[8]",
                                                  "LightData.castShadow[9]",
                                                  "LightData.castShadow[10]",
                                                  "LightData.castShadow[11]"};

  static StringArray g_pointLightShadowMapStrCache = {
      "LightData.pointLightShadowMap[0]",
      "LightData.pointLightShadowMap[1]",
      "LightData.pointLightShadowMap[2]",
      "LightData.pointLightShadowMap[3]",
      "LightData.pointLightShadowMap[4]",
      "LightData.pointLightShadowMap[5]",
      "LightData.pointLightShadowMap[6]",
      "LightData.pointLightShadowMap[7]",
      "LightData.pointLightShadowMap[8]",
      "LightData.pointLightShadowMap[9]",
      "LightData.pointLightShadowMap[10]",
      "LightData.pointLightShadowMap[11]"};

  static StringArray g_dirAndSpotLightShadowMapStrCache = {
      "LightData.dirAndSpotLightShadowMap[0]",
      "LightData.dirAndSpotLightShadowMap[1]",
      "LightData.dirAndSpotLightShadowMap[2]",
      "LightData.dirAndSpotLightShadowMap[3]",
      "LightData.dirAndSpotLightShadowMap[4]",
      "LightData.dirAndSpotLightShadowMap[5]",
      "LightData.dirAndSpotLightShadowMap[6]",
      "LightData.dirAndSpotLightShadowMap[7]",
      "LightData.dirAndSpotLightShadowMap[8]",
      "LightData.dirAndSpotLightShadowMap[9]",
      "LightData.dirAndSpotLightShadowMap[10]",
      "LightData.dirAndSpotLightShadowMap[11]"};

  static StringArray g_lightPCFSamplesStrCache = {"LightData.PCFSamples[0]",
                                                  "LightData.PCFSamples[1]",
                                                  "LightData.PCFSamples[2]",
                                                  "LightData.PCFSamples[3]",
                                                  "LightData.PCFSamples[4]",
                                                  "LightData.PCFSamples[5]",
                                                  "LightData.PCFSamples[6]",
                                                  "LightData.PCFSamples[7]",
                                                  "LightData.PCFSamples[8]",
                                                  "LightData.PCFSamples[9]",
                                                  "LightData.PCFSamples[10]",
                                                  "LightData.PCFSamples[11]"};

  static StringArray g_lightPCFRadiusStrCache = {"LightData.PCFRadius[0]",
                                                 "LightData.PCFRadius[1]",
                                                 "LightData.PCFRadius[2]",
                                                 "LightData.PCFRadius[3]",
                                                 "LightData.PCFRadius[4]",
                                                 "LightData.PCFRadius[5]",
                                                 "LightData.PCFRadius[6]",
                                                 "LightData.PCFRadius[7]",
                                                 "LightData.PCFRadius[8]",
                                                 "LightData.PCFRadius[9]",
                                                 "LightData.PCFRadius[10]",
                                                 "LightData.PCFRadius[11]"};

  static StringArray g_lightsoftShadowsStrCache = {"LightData.softShadows[0]",
                                                   "LightData.softShadows[1]",
                                                   "LightData.softShadows[2]",
                                                   "LightData.softShadows[3]",
                                                   "LightData.softShadows[4]",
                                                   "LightData.softShadows[5]",
                                                   "LightData.softShadows[6]",
                                                   "LightData.softShadows[7]",
                                                   "LightData.softShadows[8]",
                                                   "LightData.softShadows[9]",
                                                   "LightData.softShadows[10]",
                                                   "LightData.softShadows[11]"};

  static StringArray g_lightBleedingReductionStrCache = {
      "LightData.lightBleedingReduction[0]",
      "LightData.lightBleedingReduction[1]",
      "LightData.lightBleedingReduction[2]",
      "LightData.lightBleedingReduction[3]",
      "LightData.lightBleedingReduction[4]",
      "LightData.lightBleedingReduction[5]",
      "LightData.lightBleedingReduction[6]",
      "LightData.lightBleedingReduction[7]",
      "LightData.lightBleedingReduction[8]",
      "LightData.lightBleedingReduction[9]",
      "LightData.lightBleedingReduction[10]",
      "LightData.lightBleedingReduction[11]"};

} // namespace ToolKit
