#pragma once

#include "Types.h"

namespace ToolKit
{
  // Optimization caches.
  static MeshRawPtrArray g_meshCollector;

  // LightData String cache.
  static StringArray g_lightPosStrCache =
  {
    "LightData.pos[0]",
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
    "LightData.pos[11]"
  };

  static StringArray g_lightDirStrCache =
  {
    "LightData.dir[0]",
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
    "LightData.dir[11]"
  };

  static StringArray g_lightColorStrCache =
  {
    "LightData.color[0]",
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
    "LightData.color[11]"
  };

  static StringArray g_lightIntensityStrCache =
  {
    "LightData.intensity[0]",
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
    "LightData.intensity[11]"
  };

  static StringArray g_lightTypeStrCache =
  {
    "LightData.type[0]",
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
    "LightData.type[11]"
  };

  static StringArray g_lightRadiusStrCache =
  {
    "LightData.radius[0]",
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
    "LightData.radius[11]"
  };

  static StringArray g_lightOuterAngleStrCache =
  {
    "LightData.outAngle[0]",
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
    "LightData.outAngle[11]"
  };

  static StringArray g_lightInnerAngleStrCache =
  {
    "LightData.innAngle[0]",
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
    "LightData.innAngle[11]"
  };

  static StringArray g_lightprojectionViewMatrixStrCache =
  {
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
    "LightData.projectionViewMatrix[11]"
  };

  static StringArray g_lightCastShadowStrCache =
  {
    "LightData.castShadow[0]",
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
    "LightData.castShadow[11]"
  };

  static StringArray g_lightShadowBiasStrCache =
  {
    "LightData.shadowBias[0]",
    "LightData.shadowBias[1]",
    "LightData.shadowBias[2]",
    "LightData.shadowBias[3]",
    "LightData.shadowBias[4]",
    "LightData.shadowBias[5]",
    "LightData.shadowBias[6]",
    "LightData.shadowBias[7]",
    "LightData.shadowBias[8]",
    "LightData.shadowBias[9]",
    "LightData.shadowBias[10]",
    "LightData.shadowBias[11]"
  };

  static StringArray g_lightShadowMapCamFarPlaneStrCache =
  {
    "LightData.shadowMapCamFarPlane[0]",
    "LightData.shadowMapCamFarPlane[1]",
    "LightData.shadowMapCamFarPlane[2]",
    "LightData.shadowMapCamFarPlane[3]",
    "LightData.shadowMapCamFarPlane[4]",
    "LightData.shadowMapCamFarPlane[5]",
    "LightData.shadowMapCamFarPlane[6]",
    "LightData.shadowMapCamFarPlane[7]",
    "LightData.shadowMapCamFarPlane[8]",
    "LightData.shadowMapCamFarPlane[9]",
    "LightData.shadowMapCamFarPlane[10]",
    "LightData.shadowMapCamFarPlane[11]"
  };

  static StringArray g_pointLightShadowMapStrCache =
  {
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
    "LightData.pointLightShadowMap[11]"
  };

  static StringArray g_dirAndSpotLightShadowMapStrCache =
  {
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
    "LightData.dirAndSpotLightShadowMap[11]"
  };

  static StringArray g_boneTransformStrCache =
  {
    "bones[0].transform",
    "bones[1].transform",
    "bones[2].transform",
    "bones[3].transform",
    "bones[4].transform",
    "bones[5].transform",
    "bones[6].transform",
    "bones[7].transform",
    "bones[8].transform",
    "bones[9].transform",
    "bones[10].transform",
    "bones[11].transform",
    "bones[12].transform",
    "bones[13].transform",
    "bones[14].transform",
    "bones[15].transform",
    "bones[16].transform",
    "bones[17].transform",
    "bones[18].transform",
    "bones[19].transform",
    "bones[20].transform",
    "bones[21].transform",
    "bones[22].transform",
    "bones[23].transform",
    "bones[24].transform",
    "bones[25].transform",
    "bones[26].transform",
    "bones[27].transform",
    "bones[28].transform",
    "bones[29].transform",
    "bones[30].transform",
    "bones[31].transform",
    "bones[32].transform",
    "bones[33].transform",
    "bones[34].transform",
    "bones[35].transform",
    "bones[36].transform",
    "bones[37].transform",
    "bones[38].transform",
    "bones[39].transform",
    "bones[40].transform",
    "bones[41].transform",
    "bones[42].transform",
    "bones[43].transform",
    "bones[44].transform",
    "bones[45].transform",
    "bones[46].transform",
    "bones[47].transform",
    "bones[48].transform",
    "bones[49].transform",
    "bones[50].transform",
    "bones[51].transform",
    "bones[52].transform",
    "bones[53].transform",
    "bones[54].transform",
    "bones[55].transform",
    "bones[56].transform",
    "bones[57].transform",
    "bones[58].transform",
    "bones[59].transform",
    "bones[60].transform",
    "bones[61].transform",
    "bones[62].transform",
    "bones[63].transform",
    "bones[64].transform"
  };

  static StringArray g_boneBindPosStrCache =
  {
    "bones[0].bindPose",
    "bones[1].bindPose",
    "bones[2].bindPose",
    "bones[3].bindPose",
    "bones[4].bindPose",
    "bones[5].bindPose",
    "bones[6].bindPose",
    "bones[7].bindPose",
    "bones[8].bindPose",
    "bones[9].bindPose",
    "bones[10].bindPose",
    "bones[11].bindPose",
    "bones[12].bindPose",
    "bones[13].bindPose",
    "bones[14].bindPose",
    "bones[15].bindPose",
    "bones[16].bindPose",
    "bones[17].bindPose",
    "bones[18].bindPose",
    "bones[19].bindPose",
    "bones[20].bindPose",
    "bones[21].bindPose",
    "bones[22].bindPose",
    "bones[23].bindPose",
    "bones[24].bindPose",
    "bones[25].bindPose",
    "bones[26].bindPose",
    "bones[27].bindPose",
    "bones[28].bindPose",
    "bones[29].bindPose",
    "bones[30].bindPose",
    "bones[31].bindPose",
    "bones[32].bindPose",
    "bones[33].bindPose",
    "bones[34].bindPose",
    "bones[35].bindPose",
    "bones[36].bindPose",
    "bones[37].bindPose",
    "bones[38].bindPose",
    "bones[39].bindPose",
    "bones[40].bindPose",
    "bones[41].bindPose",
    "bones[42].bindPose",
    "bones[43].bindPose",
    "bones[44].bindPose",
    "bones[45].bindPose",
    "bones[46].bindPose",
    "bones[47].bindPose",
    "bones[48].bindPose",
    "bones[49].bindPose",
    "bones[50].bindPose",
    "bones[51].bindPose",
    "bones[52].bindPose",
    "bones[53].bindPose",
    "bones[54].bindPose",
    "bones[55].bindPose",
    "bones[56].bindPose",
    "bones[57].bindPose",
    "bones[58].bindPose",
    "bones[59].bindPose",
    "bones[60].bindPose",
    "bones[61].bindPose",
    "bones[62].bindPose",
    "bones[63].bindPose",
    "bones[64].bindPose"
  };

}  // namespace ToolKit
