#
# Copyright (C) 2011 The Android Open-Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

# WARNING: Everything listed here will be built on ALL platforms,
# including x86, the universal, and the SDK.  Modules must be uniquely
# named (liblights.panda), and must build everywhere, or limit themselves
# to only building on ARM if they include assembly. Individual makefiles
# are responsible for having their own logic, for fine-grained control.

LOCAL_PATH := $(call my-dir)

# if some modules are built directly from this directory (not subdirectories),
# their rules should be written here.

ifeq ($(TARGET_PRODUCT),full_erd9945_u)
include $(call all-makefiles-under,$(LOCAL_PATH))
endif

ifeq ($(TARGET_PRODUCT),full_erd9945_evt0_u)
include $(call all-makefiles-under,$(LOCAL_PATH))
endif

ifeq ($(TARGET_PRODUCT),full_erd9945_s5400_u)
include $(call all-makefiles-under,$(LOCAL_PATH))
endif

ifeq ($(TARGET_PRODUCT),full_erd9945_s6375_u)
include $(call all-makefiles-under,$(LOCAL_PATH))
endif

ifeq ($(TARGET_SOC_BASE),s5e9945)
include $(call all-makefiles-under,$(LOCAL_PATH))
include device/samsung/essi/MergeConfig.mk
endif
