#
# Script for exporting Blender models (meshes) to Colobot model files
#  (text format)
#
# Copyright (C) 2012, PPC (Polish Portal of Colobot)
#

import bpy
import struct
import array


class ExportColobot(bpy.types.Operator):
    """Exporter to Colobot text format"""
    bl_idname = "export.colobot"
    bl_label = "Export to Colobot"

    # Version of format
    FORMAT_VERSION = 1

    # TODO: set the following in some user-friendly way
    #  or better, read them from custom, per-polygon data
    # For now, you must make any changes here, as appropriate

    # Render state
    STATE = 0
    # Min & max LOD
    MIN = 0.0
    MAX = 0.0
    # Variable tex2
    VAR_TEX2 = False

    filepath = bpy.props.StringProperty(subtype="FILE_PATH")

    @classmethod
    def poll(cls, context):
        return context.object is not None

    def execute(self, context):
        self.write(context.object, context.scene)
        return {'FINISHED'}

    def invoke(self, context, event):
        context.window_manager.fileselect_add(self)
        return {'RUNNING_MODAL'}

    def write(self, object, scene):
        if (object.type != 'MESH'):
            self.report({'ERROR'}, 'Only mesh objects can be exported!')
            return

        for poly in object.data.polygons:
            if (poly.loop_total > 3):
                self.report({'ERROR'}, 'Cannot export polygons with > 3 vertices!')
                return

        file = open(self.filepath, 'w')


        file.write('# Colobot text model\n')
        file.write('\n')

        triangleCount = len(object.data.polygons) * 3

        file.write('### HEAD\n')
        file.write('version ' + str(ExportColobot.FORMAT_VERSION) + '\n')
        file.write('total_triangles ' + str(triangleCount) + '\n')
        file.write('\n')
        file.write('### TRIANGLES\n')


        for poly in object.data.polygons:

            i = 0
            for loop_index in poly.loop_indices:
                v = object.data.vertices[object.data.loops[loop_index].vertex_index]

                i = i + 1

                file.write('p' + str(i))
                file.write(' c ' + str(v.co[0]) + ' ' + str(v.co[1]) + ' ' + str(v.co[2]))
                file.write(' n ' + str(v.normal[0]) + ' ' + str(v.normal[1]) + ' ' + str(v.normal[2]))

                uv1 = array.array('f', [0.0, 0.0])
                uv2 = array.array('f', [0.0, 0.0])

                if (len(object.data.uv_layers) >= 1):
                    uv1 = object.data.uv_layers[0].data[loop_index].uv
                if (len(object.data.uv_layers) >= 2):
                    uv2 = object.data.uv_layers[1].data[loop_index].uv

                file.write(' t1 ' + str(uv1[0]) + ' ' + str(uv1[1]))
                file.write(' t2 ' + str(uv2[0]) + ' ' + str(uv2[1]))
                file.write('\n')

            mat = object.data.materials[poly.material_index]

            file.write('mat')
            file.write(' dif ' + str(mat.diffuse_color[0]) + ' ' + str(mat.diffuse_color[1]) + ' ' + str(mat.diffuse_color[2]))
            amb = scene.world.ambient_color * mat.ambient
            file.write(' amb ' + str(amb[0]) + ' ' + str(amb[1]) + ' ' + str(amb[2]))
            file.write(' spc ' + str(mat.specular_color[0]) + ' ' + str(mat.specular_color[1]) + ' ' + str(mat.specular_color[2]))
            file.write('\n')

            tex1 = ''
            tex2 = ''

            if (len(object.data.uv_textures) >= 1):
                tex1 = bpy.path.basename(object.data.uv_textures[0].data[0].image.filepath)
            if (len(object.data.uv_textures) >= 2):
                tex2 = bpy.path.basename(object.data.uv_textures[1].data[0].image.filepath)

            file.write('tex1 ' + tex1 + '\n')
            file.write('tex2 ' + tex2 + '\n')
            file.write('var_tex2 ' + 'Y' if ExportColobot.VAR_TEX2 else 'N' + '\n')
            file.write('min ' + str(ExportColobot.MIN) + '\n')
            file.write('max ' + str(ExportColobot.MAX) + '\n')
            file.write('state ' + str(ExportColobot.STATE) + '\n')
            file.write('\n')

        file.close()
        self.report({'INFO'}, 'Export OK')


# For menu item
def menu_func(self, context):
    self.layout.operator_context = 'INVOKE_DEFAULT'
    self.layout.operator(ExportColobot.bl_idname, text="Colobot (Text Format)")

# Register and add to the file selector
bpy.utils.register_class(ExportColobot)
bpy.types.INFO_MT_file_export.append(menu_func)
