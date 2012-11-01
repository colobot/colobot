#
# Script for exporting Blender models (meshes) to Colobot model files
#  (text format)
#
# Copyright (C) 2012, PPC (Polish Portal of Colobot)
#

import bpy
import struct
import array
import os
import copy

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
        return fuzzy_equal_v(self.coord, other.coord) and fuzzy_equal_v(self.normal, other.normal) and fuzzy_equal_v(self.t1, other.t1) and fuzzy_equal_v(self.t2, other.t2)

class ColobotMaterial:
    """Material as saved in Colobot model file"""
    def __init__(self):
        self.diffuse = array.array('f', [0.0, 0.0, 0.0, 0.0])
        self.ambient = array.array('f', [0.0, 0.0, 0.0, 0.0])
        self.specular = array.array('f', [0.0, 0.0, 0.0, 0.0])
        self.tex1 = ''
        self.tex2 = ''

    def __hash__(self):
        return 1

    def __eq__(self, other):
        return fuzzy_equal_v(self.diffuse, other.diffuse) and fuzzy_equal_v(self.ambient, other.ambient) and fuzzy_equal_v(self.specular, other.specular) and self.tex1 == other.tex1 and self.tex2 == other.tex2

class ColobotTriangle:
    """Triangle as saved in Colobot model file"""
    def __init__(self):
        self.p = [ColobotVertex(), ColobotVertex(), ColobotVertex()]
        self.mat = ColobotMaterial()
        self.tex1 = ''
        self.tex2 = ''
        self.var_tex2 = False
        self.state = 0
        self.min = 0.0
        self.max = 0.0

class ColobotModel:
    """Colobot model (content of model file)"""
    def __init__(self):
        self.version = 1
        self.triangles = []

    def append(self, model):
        self.triangles.extend(model.triangles)

def v3to4(vec):
    return array.array('f', [vec[0], vec[1], vec[2], 0.0])

def v4to3(vec):
    return array.array('f', [vec[0], vec[1], vec[2]])

def write_colobot_model(filename, model):
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
            file.write(' c ' + ' '.join(map(str, p.coord )))
            file.write(' n ' + ' '.join(map(str, p.normal)))
            file.write(' t1 ' + ' '.join(map(str, p.t1)))
            file.write(' t2 ' + ' '.join(map(str, p.t2)))
            file.write('\n')

        file.write('mat')
        file.write(' dif ' + ' '.join(map(str, t.mat.diffuse)))
        file.write(' amb ' + ' '.join(map(str, t.mat.ambient)))
        file.write(' spc ' + ' '.join(map(str, t.mat.specular)))
        file.write('\n')

        file.write('tex1 ' + t.tex1 + '\n')
        file.write('tex2 ' + t.tex2 + '\n')
        file.write('var_tex2 ' + ( 'Y' if t.var_tex2 else 'N' + '\n' ) )
        file.write('min ' + str(t.min) + '\n')
        file.write('max ' + str(t.max) + '\n')
        file.write('state ' + str(t.state) + '\n')
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
        raise ColobotError('Invalid header')
    model.version = int(tokens[1])
    if (model.version != 1):
        raise ColobotError('Unknown model file version')

    tokens, index = token_next_line(lines, index)
    if (tokens[0] != 'total_triangles'):
        raise ColobotError('Invalid header')
    numTriangles = int(tokens[1])

    for i in range(0, numTriangles):
        t = ColobotTriangle()

        tokens, index = token_next_line(lines, index)
        if (tokens[0] != 'p1'):
            raise ColobotError('Invalid triangle')
        t.p[0] = read_colobot_vertex(tokens)

        tokens, index = token_next_line(lines, index)
        if (tokens[0] != 'p2'):
            raise ColobotError('Invalid triangle')
        t.p[1] = read_colobot_vertex(tokens)

        tokens, index = token_next_line(lines, index)
        if (tokens[0] != 'p3'):
            raise ColobotError('Invalid triangle')
        t.p[2] = read_colobot_vertex(tokens)

        tokens, index = token_next_line(lines, index)
        if (tokens[0] != 'mat'):
            raise ColobotError('Invalid triangle')
        t.mat = read_colobot_material(tokens)

        tokens, index = token_next_line(lines, index)
        if (tokens[0] != 'tex1'):
            raise ColobotError('Invalid triangle')
        if (len(tokens) > 1):
            t.tex1 = tokens[1]

        tokens, index = token_next_line(lines, index)
        if (tokens[0] != 'tex2'):
            raise ColobotError('Invalid triangle')
        if (len(tokens) > 1):
            t.tex2 = tokens[1]

        tokens, index = token_next_line(lines, index)
        if (tokens[0] != 'var_tex2'):
            raise ColobotError('Invalid triangle')
        t.var_tex2 = tokens[1] == 'Y'

        tokens, index = token_next_line(lines, index)
        if (tokens[0] != 'min'):
            raise ColobotError('Invalid triangle')
        t.min = float(tokens[1])

        tokens, index = token_next_line(lines, index)
        if (tokens[0] != 'max'):
            raise ColobotError('Invalid triangle')
        t.max = float(tokens[1])

        tokens, index = token_next_line(lines, index)
        if (tokens[0] != 'state'):
            raise ColobotError('Invalid triangle')
        t.state = int(tokens[1])

        model.triangles.append(t)

    return model

def mesh_to_colobot_model(mesh, scene, defaults):
    model = ColobotModel()

    if (mesh.type != 'MESH'):
        raise ColobotError('Only mesh meshs can be exported')

    for poly in mesh.data.polygons:
        if (poly.loop_total > 3):
            raise ColobotError('Cannot export polygons with > 3 vertices!')

    for i, poly in enumerate(mesh.data.polygons):
        t = ColobotTriangle()
        j = 0
        for loop_index in poly.loop_indices:
            v = mesh.data.vertices[mesh.data.loops[loop_index].vertex_index]

            t.p[j].coord = copy.copy(v.co)
            t.p[j].normal = copy.copy(v.normal)

            if (len(mesh.data.uv_layers) >= 1):
                t.p[j].t1 = copy.copy(mesh.data.uv_layers[0].data[loop_index].uv)
                t.p[j].t1[1] = 1.0 - t.p[j].t1[1]
            if (len(mesh.data.uv_layers) >= 2):
                t.p[j].t2 = copy.copy(mesh.data.uv_layers[1].data[loop_index].uv)
                t.p[j].t2[1] = 1.0 - t.p[j].t2[1]

            j = j + 1

        mat = mesh.data.materials[poly.material_index]
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

        t.var_tex2 = mesh.get('var_tex2', defaults['var_tex2'])
        t.state = mesh.get('state', defaults['state'])
        t.min = mesh.get('min', defaults['min'])
        t.max = mesh.get('max', defaults['max'])

        model.triangles.append(t)

    return model


def colobot_model_to_mesh(model, mesh_name, texture_dir):
    mesh = bpy.data.meshes.new(name=mesh_name)

    vertex_set = set()

    for t in model.triangles:
        for i in range(0, 3):
            vertex_set.add(t.p[i])

    vertex_list = list(vertex_set)

    mat_set = set()

    for t in model.triangles:
        mat = t.mat
        mat.tex1 = t.tex1
        mat.tex2 = t.tex2
        mat_set.add(mat)

    mat_list = list(mat_set)

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

    for i, m in enumerate(mat_list):
        material = bpy.data.materials.new(name=mesh_name + '_mat_' + str(i+1))
        material.diffuse_color = v4to3(m.diffuse)
        material.ambient = (m.ambient[0] + m.ambient[1] + m.ambient[2]) / 3.0
        material.alpha = (m.diffuse[3] + m.ambient[3]) / 2.0
        material.specular_color = v4to3(m.specular)
        material.specular_alpha = m.specular[3]

        mesh.materials.append(material)

    mesh.tessfaces.add(len(model.triangles))

    for i, f in enumerate(mesh.tessfaces):
        t = model.triangles[i]
        mat = t.mat
        mat.tex1 = t.tex1
        mat.tex2 = t.tex2
        f.material_index = mat_list.index(mat)
        for i in range(0, 3):
            f.vertices[i] = vertex_list.index(t.p[i])

    if uv1map:
        uvlay1 = mesh.tessface_uv_textures.new(name='UV_1')
        for i, f in enumerate(uvlay1.data):
            f.uv1[0] = model.triangles[i].p[0].t1[0]
            f.uv1[1] = 1.0 - model.triangles[i].p[0].t1[1]
            f.uv2[0] = model.triangles[i].p[1].t1[0]
            f.uv2[1] = 1.0 - model.triangles[i].p[1].t1[1]
            f.uv3[0] = model.triangles[i].p[2].t1[0]
            f.uv3[1] = 1.0 - model.triangles[i].p[2].t1[1]

    if uv2map:
        uvlay2 = mesh.tessface_uv_textures.new(name='UV_2')
        for i, f in enumerate(uvlay2.data):
            f.uv1[0] = model.triangles[i].p[0].t2[0]
            f.uv1[1] = 1.0 - model.triangles[i].p[0].t2[1]
            f.uv2[0] = model.triangles[i].p[1].t2[0]
            f.uv2[1] = 1.0 - model.triangles[i].p[1].t2[1]
            f.uv3[0] = model.triangles[i].p[2].t2[0]
            f.uv3[1] = 1.0 - model.triangles[i].p[2].t2[1]

    def load_tex(name):
        import os
        import sys
        from bpy_extras.image_utils import load_image

        if (name == ''):
            return None, None

        encoding = sys.getfilesystemencoding()
        image = load_image(name, texture_dir, recursive=True, place_holder=True)
        texture = None
        if image:
            name = bpy.path.display_name_from_filepath(name)
            texture = bpy.data.textures.new(name=name, type='IMAGE')
            texture.image = image
        return image, texture

    for i, m in enumerate(mat_list):

        image1, tex1 = load_tex(m.tex1)
        if image1:
            mtex = mesh.materials[i].texture_slots.add()
            mtex.texture = tex1
            mtex.texture_coords = 'UV'
            mtex.uv_layer = 'UV_1'
            mtex.use_map_color_diffuse = True

            for j, face in enumerate(mesh.uv_textures[0].data):
                if (model.triangles[j].tex1 == m.tex1):
                    face.image = image1

        image2, tex2 = load_tex(m.tex2)
        if image2:
            mtex = mesh.materials[i].texture_slots.add()
            mtex.texture = tex2
            mtex.texture_coords = 'UV'
            mtex.uv_layer = 'UV_2'
            mtex.use_map_color_diffuse = True

            for face in mesh.uv_textures[1].data:
                if (model.triangles[j].tex2 == m.tex2):
                    face.image = image2

    mesh.validate()
    mesh.update()

    return mesh

class ExportColobot(bpy.types.Operator):
    """Exporter to Colobot text format"""
    bl_idname = "export.colobot"
    bl_label = "Export to Colobot"

    # TODO: set the following in a UI dialog or panel

    # Variable tex2
    DEFAULT_VAR_TEX2 = False
    # Min & max LOD
    DEFAULT_MIN = 0.0
    DEFAULT_MAX = 0.0
    # Render state
    DEFAULT_STATE = 0

    filepath = bpy.props.StringProperty(subtype="FILE_PATH")

    @classmethod
    def poll(cls, context):
        return context.object is not None

    def execute(self, context):
        defaults = {
            'var_tex2': self.DEFAULT_VAR_TEX2,
            'min': self.DEFAULT_MIN,
            'max': self.DEFAULT_MAX,
            'state': self.DEFAULT_STATE }
        try:
            model = mesh_to_colobot_model(context.object, context.scene, defaults)
            write_colobot_model(self.filepath, model)
        except ColobotError as e:
            self.report({'ERROR'}, e.args[0])
            return {'FINISHED'}

        self.report({'INFO'}, 'Export OK')
        return {'FINISHED'}

    def invoke(self, context, event):
        context.window_manager.fileselect_add(self)
        return {'RUNNING_MODAL'}


# For menu item
def export_menu_func(self, context):
    self.layout.operator_context = 'INVOKE_DEFAULT'
    self.layout.operator(ExportColobot.bl_idname, text="Colobot (Text Format)")

# Register and add to the file selector
bpy.utils.register_class(ExportColobot)
bpy.types.INFO_MT_file_export.append(export_menu_func)



class ImportColobot(bpy.types.Operator):
    """Importer from Colobot text format"""
    bl_idname = "import.colobot"
    bl_label = "Import from Colobot"

    filepath = bpy.props.StringProperty(subtype="FILE_PATH")

    @classmethod
    def poll(cls, context):
        return True

    def execute(self, context):
        try:
            model = read_colobot_model(self.filepath)
            mesh = colobot_model_to_mesh(model, 'ColobotMesh', os.path.dirname(self.filepath))
            obj = bpy.data.objects.new('ColobotMesh', mesh)
            bpy.context.scene.objects.link(obj)
            bpy.context.scene.objects.active = obj
            obj.select = True
        except ColobotError as e:
            self.report({'ERROR'}, e.args[0])
            return {'FINISHED'}

        self.report({'INFO'}, 'Import OK')
        return {'FINISHED'}

    def invoke(self, context, event):
        context.window_manager.fileselect_add(self)
        return {'RUNNING_MODAL'}


# For menu item
def import_menu_func(self, context):
    self.layout.operator_context = 'INVOKE_DEFAULT'
    self.layout.operator(ImportColobot.bl_idname, text="Colobot (Text Format)")

# Register and add to the file selector
bpy.utils.register_class(ImportColobot)
bpy.types.INFO_MT_file_import.append(import_menu_func)
