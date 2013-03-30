#
# Script for exporting Blender models (meshes) to Colobot model files
#  (text format)
#
# Copyright (C) 2012, PPC (Polish Portal of Colobot)
#

bl_info = {
    "name": "Colobot Model Format (.txt)",
    "author": "PPC (Polish Portal of Colobot)",
    "version": (0, 0, 2),
    "blender": (2, 6, 4),
    "location": "File > Export > Colobot (.txt)",
    "description": "Export Colobot Model Format (.txt)",
    "warning": "",
    "wiki_url": "http://colobot.info"\
        "",
    "tracker_url": ""\
        "",
    "category": "Import-Export"}

import bpy
import struct
import array
import os
import copy
import math


##
# Data types & helper functions
##

FUZZY_TOLERANCE = 1e-5

class ColobotError(Exception):
    """Exception in I/O operations"""
    def __init__(self, value):
        self.value = value
    def __str__(self):
        return repr(self.value)

def fuzzy_equal_v(v1, v2):
    for i in range(len(v1)):
        if abs(v1[i] - v2[i]) > FUZZY_TOLERANCE:
            return False
    return True

class ColobotVertex:
    """Vertex as saved in Colobot model file"""
    def __init__(self):
        self.coord = array.array('f', [0.0, 0.0, 0.0])
        self.normal = array.array('f', [0.0, 0.0, 0.0])
        self.t1 = array.array('f', [0.0, 0.0])
        self.t2 = array.array('f', [0.0, 0.0])

    def __hash__(self):
        return 1

    def __eq__(self, other):
        return (fuzzy_equal_v(self.coord, other.coord) and
               fuzzy_equal_v(self.normal, other.normal) and
               fuzzy_equal_v(self.t1, other.t1) and
               fuzzy_equal_v(self.t2, other.t2))

class ColobotMaterial:
    """Material as saved in Colobot model file"""
    def __init__(self):
        self.diffuse = array.array('f', [0.0, 0.0, 0.0, 0.0])
        self.ambient = array.array('f', [0.0, 0.0, 0.0, 0.0])
        self.specular = array.array('f', [0.0, 0.0, 0.0, 0.0])
        self.tex1 = ''
        self.tex2 = ''
        self.var_tex2 = False
        self.state = 0

    def __hash__(self):
        return 1

    def __eq__(self, other):
        return (fuzzy_equal_v(self.diffuse, other.diffuse) and
               fuzzy_equal_v(self.ambient, other.ambient) and
               fuzzy_equal_v(self.specular, other.specular) and
               self.tex1 == other.tex1 and
               self.tex2 == other.tex2 and
               self.var_tex2 == other.var_tex2 and
               self.state == other.state)

class ColobotTexPair:
    """Pair of 2 textures"""
    def __init__(self):
        self.tex1 = ''
        self.tex2 = ''

    def __hash__(self):
        return 1

    def __eq__(self, other):
        return self.tex1 == other.tex1 and self.tex2 == other.tex2

class ColobotTriangle:
    """Triangle as saved in Colobot model file"""
    def __init__(self):
        self.p = [ColobotVertex(), ColobotVertex(), ColobotVertex()]
        self.mat = ColobotMaterial()
        self.lod_level = 0

class ColobotModel:
    """Colobot model (content of model file)"""
    def __init__(self):
        self.version = 1
        self.triangles = []

    def get_lod_level_list(self):
        lod_level_set = set()
        for t in self.triangles:
            lod_level_set.add(t.lod_level)

        return list(lod_level_set)

    def get_tex_pair_list(self):
        tex_pair_set = set()
        for t in self.triangles:
            tex_pair = ColobotTexPair()
            tex_pair.tex1 = t.mat.tex1
            tex_pair.tex2 = t.mat.tex2
            tex_pair_set.add(tex_pair)

        return list(tex_pair_set)

    def get_triangle_list(self, lod_level):
        triangles = []
        for t in self.triangles:
            if (t.lod_level == lod_level):
                triangles.append(t)

        return triangles

    def get_vertex_list(self, lod_level):
        vertex_set = set()

        for t in self.triangles:
            if (t.lod_level == lod_level):
                for i in range(0, 3):
                    vertex_set.add(t.p[i])

        return list(vertex_set)

    def get_material_list(self, lod_level):
        material_set = set()

        for t in self.triangles:
            if (t.lod_level == lod_level):
                material_set.add(t.mat)

        return list(material_set)

def v3to4(vec):
    return array.array('f', [vec[0], vec[1], vec[2], 0.0])

def v4to3(vec):
    return array.array('f', [vec[0], vec[1], vec[2]])


##
# Model file input/output
##

def write_colobot_model(filename, model):
    float_format = "{:g}".format

    file = open(filename, 'w')

    file.write('# Colobot text model\n')
    file.write('\n')

    file.write('### HEAD\n')
    file.write('version ' + str(model.version) + '\n')
    file.write('total_triangles ' + str(len(model.triangles)) + '\n')
    file.write('\n')
    file.write('### TRIANGLES\n')

    for t in model.triangles:
        for i in range(0, 3):
            p = t.p[i]
            file.write('p' + str(i+1))
            file.write(' c ' + ' '.join(map(float_format, p.coord )))
            file.write(' n ' + ' '.join(map(float_format, p.normal)))
            file.write(' t1 ' + ' '.join(map(float_format, p.t1)))
            file.write(' t2 ' + ' '.join(map(float_format, p.t2)))
            file.write('\n')

        file.write('mat')
        file.write(' dif ' + ' '.join(map(float_format, t.mat.diffuse)))
        file.write(' amb ' + ' '.join(map(float_format, t.mat.ambient)))
        file.write(' spc ' + ' '.join(map(float_format, t.mat.specular)))
        file.write('\n')

        file.write('tex1 ' + t.mat.tex1 + '\n')
        file.write('tex2 ' + t.mat.tex2 + '\n')
        file.write('var_tex2 ' + ( 'Y' if t.mat.var_tex2 else 'N' + '\n' ) )
        file.write('lod_level ' + str(t.lod_level) + '\n')
        file.write('state ' + str(t.mat.state) + '\n')
        file.write('\n')

    file.close()

def token_next_line(lines, index):
    while (index < len(lines)):
        line = lines[index]
        index = index + 1
        if (not (len(line) == 0 or line[0] == '#' or line[0] == '\n') ):
            return ( line.split(), index)

    raise ColobotError('Unexpected EOF')

def read_colobot_vertex(tokens):
    vertex = ColobotVertex()

    if (tokens[1] != 'c'):
        raise ColobotError('Invalid vertex')
    vertex.coord[0] = float(tokens[2])
    vertex.coord[1] = float(tokens[3])
    vertex.coord[2] = float(tokens[4])

    if (tokens[5] != 'n'):
        raise ColobotError('Invalid vertex')
    vertex.normal[0] = float(tokens[6])
    vertex.normal[1] = float(tokens[7])
    vertex.normal[2] = float(tokens[8])

    if (tokens[9] != 't1'):
        raise ColobotError('Invalid vertex')
    vertex.t1[0] = float(tokens[10])
    vertex.t1[1] = float(tokens[11])

    if (tokens[12] != 't2'):
        raise ColobotError('Invalid vertex')
    vertex.t2[0] = float(tokens[13])
    vertex.t2[1] = float(tokens[14])

    return vertex

def read_colobot_material(tokens):
    material = ColobotMaterial()

    if (tokens[1] != 'dif'):
        raise ColobotError('Invalid material')
    material.diffuse[0] = float(tokens[2])
    material.diffuse[1] = float(tokens[3])
    material.diffuse[2] = float(tokens[4])
    material.diffuse[3] = float(tokens[5])

    if (tokens[6] != 'amb'):
        raise ColobotError('Invalid material')
    material.ambient[0] = float(tokens[7])
    material.ambient[1] = float(tokens[8])
    material.ambient[2] = float(tokens[9])
    material.ambient[3] = float(tokens[10])

    if (tokens[11] != 'spc'):
        raise ColobotError('Invalid material')
    material.specular[0] = float(tokens[12])
    material.specular[1] = float(tokens[13])
    material.specular[2] = float(tokens[14])
    material.specular[3] = float(tokens[15])

    return material

def read_colobot_model(filename):
    model = ColobotModel()

    file = open(filename, 'r')
    lines = file.readlines()
    file.close()

    index = 0
    numTriangles = 0

    tokens, index = token_next_line(lines, index)
    if (tokens[0] != 'version'):
        raise ColobotError("Invalid header", "version")
    model.version = int(tokens[1])
    if (model.version != 1):
        raise ColobotError("Unknown model file version")

    tokens, index = token_next_line(lines, index)
    if (tokens[0] != 'total_triangles'):
        raise ColobotError("Invalid header", "total_triangles")
    numTriangles = int(tokens[1])

    for i in range(0, numTriangles):
        t = ColobotTriangle()

        tokens, index = token_next_line(lines, index)
        if (tokens[0] != 'p1'):
            raise ColobotError("Invalid triangle", "p1")
        t.p[0] = read_colobot_vertex(tokens)

        tokens, index = token_next_line(lines, index)
        if (tokens[0] != 'p2'):
            raise ColobotError("Invalid triangle", "p2")
        t.p[1] = read_colobot_vertex(tokens)

        tokens, index = token_next_line(lines, index)
        if (tokens[0] != 'p3'):
            raise ColobotError("Invalid triangle", "p3")
        t.p[2] = read_colobot_vertex(tokens)

        tokens, index = token_next_line(lines, index)
        if (tokens[0] != 'mat'):
            raise ColobotError("Invalid triangle", "mat")
        t.mat = read_colobot_material(tokens)

        tokens, index = token_next_line(lines, index)
        if (tokens[0] != 'tex1'):
            raise ColobotError("Invalid triangle", "tex1")
        if (len(tokens) > 1):
            t.mat.tex1 = tokens[1]

        tokens, index = token_next_line(lines, index)
        if (tokens[0] != 'tex2'):
            raise ColobotError("Invalid triangle", "tex2")
        if (len(tokens) > 1):
            t.mat.tex2 = tokens[1]

        tokens, index = token_next_line(lines, index)
        if (tokens[0] != 'var_tex2'):
            raise ColobotError("Invalid triangle", "var_tex2")
        t.mat.var_tex2 = tokens[1] == 'Y'

        tokens, index = token_next_line(lines, index)
        if (tokens[0] != 'lod_level'):
            raise ColobotError("Invalid triangle", "lod_level")
        t.lod_level = int(tokens[1])

        tokens, index = token_next_line(lines, index)
        if (tokens[0] != 'state'):
            raise ColobotError("Invalid triangle", "state")
        t.mat.state = int(tokens[1])

        model.triangles.append(t)

    return model


##
# Mesh conversion functions
##

def append_obj_to_colobot_model(obj, model, scene, defaults):

    if (obj.type != 'MESH'):
        raise ColobotError('Only mesh meshs can be exported')

    for poly in obj.data.polygons:
        if (poly.loop_total > 3):
            raise ColobotError('Cannot export polygons with > 3 vertices!')

    for i, poly in enumerate(obj.data.polygons):
        t = ColobotTriangle()
        j = 0
        for loop_index in poly.loop_indices:
            v = obj.data.vertices[obj.data.loops[loop_index].vertex_index]

            t.p[j].coord = copy.copy(v.co)
            t.p[j].normal = copy.copy(v.normal)

            if (len(obj.data.uv_layers) >= 1):
                t.p[j].t1 = copy.copy(obj.data.uv_layers[0].data[loop_index].uv)
                t.p[j].t1[1] = 1.0 - t.p[j].t1[1]
            if (len(obj.data.uv_layers) >= 2):
                t.p[j].t2 = copy.copy(obj.data.uv_layers[1].data[loop_index].uv)
                t.p[j].t2[1] = 1.0 - t.p[j].t2[1]

            j = j + 1

        mat = obj.data.materials[poly.material_index]
        t.mat.diffuse = v3to4(mat.diffuse_color)
        t.mat.diffuse[3] = mat.alpha
        t.mat.ambient = v3to4(scene.world.ambient_color * mat.ambient)
        t.mat.ambient[3] = mat.alpha
        t.mat.specular = v3to4(mat.specular_color)
        t.mat.specular[3] = mat.specular_alpha

        if (mat.texture_slots[0] != None):
            t.tex1 = bpy.path.basename(mat.texture_slots[0].texture.image.filepath)
        if (mat.texture_slots[1] != None):
            t.tex2 = bpy.path.basename(mat.texture_slots[1].texture.image.filepath)

        t.var_tex2 = mat.get('var_tex2', defaults['var_tex2'])
        t.state = mat.get('state', defaults['state'])

        t.lod_level = int(obj.data.get('lod_level', defaults['lod_level']))

        model.triangles.append(t)


def colobot_model_to_meshes(model, base_mesh_name, texture_dir):
    def load_tex(name):
        import os
        import sys
        from bpy_extras.image_utils import load_image

        if (name == ''):
            return None, None

        image = load_image(name, texture_dir, recursive=True, place_holder=True)
        texture = None
        if image:
            name = bpy.path.display_name_from_filepath(name)
            texture = bpy.data.textures.new(name=name, type='IMAGE')
            texture.image = image
        return image, texture

    class Texture:
        def __init__(self):
            self.image1 = None
            self.image2 = None
            self.tex1 = None
            self.tex2 = None

    tex_dict = dict()
    tex_pair_list = model.get_tex_pair_list()
    for tex_pair in tex_pair_list:
        tex_object = Texture()
        tex_object.image1, tex_object.tex1 = load_tex(tex_pair.tex1)
        tex_object.image2, tex_object.tex2 = load_tex(tex_pair.tex2)
        tex_dict[tex_pair] = tex_object

    meshes = []

    index = 0
    lod_levels = model.get_lod_level_list()
    for lod_level in lod_levels:
        index = index + 1
        mesh = bpy.data.meshes.new(name=base_mesh_name + str(index))

        triangle_list = model.get_triangle_list(lod_level)
        vertex_list = model.get_vertex_list(lod_level)
        material_list = model.get_material_list(lod_level)

        uv1map = False
        uv2map = False

        zero_t = array.array('f', [0.0, 0.0])

        for v in vertex_list:
            if ((not uv1map) and (v.t1 != zero_t)):
                uv1map = True
            if ((not uv2map) and (v.t2 != zero_t)):
                uv2map = True

        mesh.vertices.add(len(vertex_list))

        for i, v in enumerate(mesh.vertices):
            v.co = copy.copy(vertex_list[i].coord)
            v.normal = copy.copy(vertex_list[i].normal)

        for i, m in enumerate(material_list):
            material = bpy.data.materials.new(name=base_mesh_name + str(index) + '_mat_' + str(i+1))
            material.diffuse_color = v4to3(m.diffuse)
            material.ambient = (m.ambient[0] + m.ambient[1] + m.ambient[2]) / 3.0
            material.alpha = (m.diffuse[3] + m.ambient[3]) / 2.0
            material.specular_color = v4to3(m.specular)
            material.specular_alpha = m.specular[3]

            material.var_tex2 = m.var_tex2
            material.state = m.state

            mesh.materials.append(material)

        mesh.tessfaces.add(len(triangle_list))

        for i, f in enumerate(mesh.tessfaces):
            t = triangle_list[i]
            f.material_index = material_list.index(t.mat)
            for i in range(0, 3):
                f.vertices[i] = vertex_list.index(t.p[i])

        if uv1map:
            uvlay1 = mesh.tessface_uv_textures.new(name='UV_1')
            for i, f in enumerate(uvlay1.data):
                f.uv1[0] = triangle_list[i].p[0].t1[0]
                f.uv1[1] = 1.0 - triangle_list[i].p[0].t1[1]
                f.uv2[0] = triangle_list[i].p[1].t1[0]
                f.uv2[1] = 1.0 - triangle_list[i].p[1].t1[1]
                f.uv3[0] = triangle_list[i].p[2].t1[0]
                f.uv3[1] = 1.0 - triangle_list[i].p[2].t1[1]

        if uv2map:
            uvlay2 = mesh.tessface_uv_textures.new(name='UV_2')
            for i, f in enumerate(uvlay2.data):
                f.uv1[0] = triangle_list[i].p[0].t2[0]
                f.uv1[1] = 1.0 - triangle_list[i].p[0].t2[1]
                f.uv2[0] = triangle_list[i].p[1].t2[0]
                f.uv2[1] = 1.0 - triangle_list[i].p[1].t2[1]
                f.uv3[0] = triangle_list[i].p[2].t2[0]
                f.uv3[1] = 1.0 - triangle_list[i].p[2].t2[1]

        for i, m in enumerate(material_list):
            tex_pair = ColobotTexPair()
            tex_pair.tex1 = m.tex1
            tex_pair.tex2 = m.tex2
            tex_object = tex_dict[tex_pair]

            if tex_object and tex_object.image1:
                mtex = mesh.materials[i].texture_slots.add()
                mtex.texture = tex_object.tex1
                mtex.texture_coords = 'UV'
                mtex.uv_layer = 'UV_1'
                mtex.use_map_color_diffuse = True

                for j, face in enumerate(mesh.uv_textures[0].data):
                    if (triangle_list[j].tex1 == m.tex1):
                        face.image = tex_object.image1

            if tex_object and tex_object.image2:
                mtex = mesh.materials[i].texture_slots.add()
                mtex.texture = tex_object.tex2
                mtex.texture_coords = 'UV'
                mtex.uv_layer = 'UV_2'
                mtex.use_map_color_diffuse = True

                for j, face in enumerate(mesh.uv_textures[1].data):
                    if (triangle_list[j].tex2 == m.tex2):
                        face.image = tex_object.image2

        mesh.lod_level = str(lod_level)

        mesh.validate()
        mesh.update()

        meshes.append(mesh)

    return meshes


##
# Export UI dialog & operator
##

EXPORT_FILEPATH = ''

class ExportColobotDialog(bpy.types.Operator):
    bl_idname = 'object.export_colobot_dialog'
    bl_label = "Dialog for Colobot export"

    mode = bpy.props.EnumProperty(
        name="Mode",
        items = [('overwrite', "Overwrite", "Overwrite existing model triangles"),
                 ('append', "Append", "Append triangles to existing model")],
                 default='overwrite')

    default_lod_level = bpy.props.EnumProperty(
        name="Default LOD level",
        items = [('0', "Constant", "Constant (always visible)"),
                 ('1', "Low", "Low (visible at furthest distance)"),
                 ('2', "Medium", "Medium (visible at medium distance)"),
                 ('3', "High", "High (visible at closest distance)")],
                 default='0')

    default_var_tex2 = bpy.props.BoolProperty(name="Default variable 2nd texture", default=False)

    default_state = bpy.props.IntProperty(name="Default state", default=0)

    def execute(self, context):
        global EXPORT_FILEPATH
        try:
            defaults = { 'lod_level': self.default_lod_level,
                         'var_tex2': self.default_var_tex2,
                         'state': self.default_state }

            model = ColobotModel()
            if (self.mode == 'append'):
                model = read_colobot_model(EXPORT_FILEPATH)

            for obj in context.selected_objects:
                rot = obj.rotation_euler
                rot[0] = rot[0] + math.radians(270)
                obj.rotation_euler = rot

                append_obj_to_colobot_model(obj, model, context.scene, defaults)

                rot = obj.rotation_euler
                rot[0] = rot[0] + math.radians(90)
                obj.rotation_euler = rot

            write_colobot_model(EXPORT_FILEPATH, model)

        except ColobotError as e:
            self.report({'ERROR'}, e.args.join(": "))
            return {'FINISHED'}

        self.report({'INFO'}, 'Export OK')
        return {'FINISHED'}

    def invoke(self, context, event):
        context.window_manager.invoke_props_dialog(self, width=500)
        return {'RUNNING_MODAL'}


class ExportColobot(bpy.types.Operator):
    """Exporter to Colobot text format"""
    bl_idname = "export.colobot"
    bl_label = "Export to Colobot"

    filepath = bpy.props.StringProperty(subtype="FILE_PATH")

    @classmethod
    def poll(cls, context):
        return context.object is not None

    def execute(self, context):
        global EXPORT_FILEPATH
        EXPORT_FILEPATH = self.filepath
        bpy.ops.object.export_colobot_dialog('INVOKE_DEFAULT')
        return {'FINISHED'}

    def invoke(self, context, event):
        context.window_manager.fileselect_add(self)
        return {'RUNNING_MODAL'}


##
# Import UI dialog & operator
#

IMPORT_FILEPATH = ''

class ImportColobotDialog(bpy.types.Operator):
    bl_idname = 'object.import_colobot_dialog'
    bl_label = "Dialog for Colobot import"

    lod_separate_layers = bpy.props.BoolProperty(name="LOD levels to separate layers", default=True)
    texture_dir = bpy.props.StringProperty(name="Texture directory", subtype="DIR_PATH")

    def execute(self, context):
        global IMPORT_FILEPATH
        try:
            texture_dir = self.texture_dir
            if (texture_dir == ""):
                texture_dir = os.path.dirname(IMPORT_FILEPATH)

            model = read_colobot_model(IMPORT_FILEPATH)
            meshes = colobot_model_to_meshes(model, 'ColobotMesh_', texture_dir)
            index = 0
            for mesh in meshes:
                index = index + 1
                obj = bpy.data.objects.new('ColobotMesh_' + str(index), mesh)

                rot = obj.rotation_euler
                rot[0] = rot[0] + math.radians(90)
                obj.rotation_euler = rot

                bpy.context.scene.objects.link(obj)
                bpy.context.scene.objects.active = obj
                obj.select = True

                # TODO: doesn't seem to work...
                if (self.lod_separate_layers):
                    layers = obj.layers
                    for i in range(0, len(layers)):
                        layers[i] = int(mesh.lod_level) == i
                    obj.layers = layers

        except ColobotError as e:
            self.report({'ERROR'}, e.args.join(": "))
            return {'FINISHED'}

        self.report({'INFO'}, 'Import OK')
        return {'FINISHED'}

    def invoke(self, context, event):
        context.window_manager.invoke_props_dialog(self, width=500)
        return {'RUNNING_MODAL'}


class ImportColobot(bpy.types.Operator):
    """Importer from Colobot text format"""
    bl_idname = "import.colobot"
    bl_label = "Import from Colobot"

    filepath = bpy.props.StringProperty(subtype="FILE_PATH")

    @classmethod
    def poll(cls, context):
        return True

    def execute(self, context):
        global IMPORT_FILEPATH
        IMPORT_FILEPATH = self.filepath
        bpy.ops.object.import_colobot_dialog('INVOKE_DEFAULT')
        return {'FINISHED'}

    def invoke(self, context, event):
        context.window_manager.fileselect_add(self)
        return {'RUNNING_MODAL'}


##
# Registration
##

# Callback functions for menu items
def export_menu_func(self, context):
    self.layout.operator_context = 'INVOKE_DEFAULT'
    self.layout.operator(ExportColobot.bl_idname, text="Colobot (Text Format)")

def import_menu_func(self, context):
    self.layout.operator_context = 'INVOKE_DEFAULT'
    self.layout.operator(ImportColobot.bl_idname, text="Colobot (Text Format)")

# Custom properties for materials
def register_material_props():
    bpy.types.Mesh.lod_level = bpy.props.EnumProperty(name="LOD level",
        items = [('0', "Constant", "Constant (always visible)"),
                 ('1', "Low", "Low (visible at furthest distance)"),
                 ('2', "Medium", "Medium (visible at medium distance)"),
                 ('3', "High", "High (visible at closest distance)")])
    bpy.types.Material.var_tex2 = bpy.props.BoolProperty(name="Variable 2nd texture", description="2nd texture shall be set to dirtyXX.png")
    bpy.types.Material.state = bpy.props.IntProperty(name="State", description="Engine render state")

# Add-on registration
def register():
    bpy.utils.register_module(__name__)

    register_material_props()

    bpy.types.INFO_MT_file_export.append(export_menu_func)
    bpy.types.INFO_MT_file_import.append(import_menu_func)
