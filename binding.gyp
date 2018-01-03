{
	'variables': {
		'platform'      : '<(OS)',
		'opengl_root'   : '<!(node -e "console.log(require(\'node-deps-opengl-raub\').root)")',
		'opengl_include': '<(opengl_root)/include',
		'opengl_bin'    : '<!(node -e "console.log(require(\'node-deps-opengl-raub\').bin)")',
	},
	'conditions': [
		['platform == "mac"', { 'variables': { 'platform': 'darwin' } }],
		['platform == "win"', { 'variables': { 'platform': 'windows'  } }],
	],
	'targets': [
		{
			'target_name': 'glfw',
			'defines': [ 'VERSION=0.4.6' ],
			'sources': [ 'src/glfw.cpp' ],
			'include_dirs': [
				'<!(node -e "require(\'nan\')")',
				'<(opengl_include)',
			],
			'library_dirs': [ '<(opengl_bin)' ],
			'conditions': [
				[
					'OS=="linux"',
					{
						'libraries': ['-lfreeimage', '-lglfw', '-lglew', '-lGL', '-lXrandr']
					}
				],
				[
					'OS=="mac"',
					{
						'include_dirs': [ '<!@(pkg-config glfw3 glew --cflags-only-I | sed s/-I//g)' ],
						'libraries'   : [ '<!@(pkg-config --libs glfw3 glew)', '-framework OpenGL' ],
						'library_dirs': [ '/usr/local/lib' ],
					}
				],
				[
					'OS=="win"',
					{
						'libraries': [ 'FreeImage.lib', 'glfw3dll.lib', 'glew32.lib', 'opengl32.lib' ],
						'defines' : [
							'WIN32_LEAN_AND_MEAN',
							'VC_EXTRALEAN'
						],
						'msvs_settings' : {
							'VCCLCompilerTool' : {
								'AdditionalOptions' : [
									'/O2','/Oy','/GL','/GF','/Gm-',
									'/EHsc','/MT','/GS','/Gy','/GR-','/Gd',
								]
							},
							'VCLinkerTool' : {
								'AdditionalOptions' : ['/OPT:REF','/OPT:ICF','/LTCG']
							},
						},
					},
				],
			],
		},
		
		{
			'target_name'  : 'make_directory',
			'type'         : 'none',
			'dependencies' : ['glfw'],
			'actions'      : [{
				'action_name' : 'Directory created.',
				'inputs'      : [],
				'outputs'     : ['build'],
				'action'      : ['mkdir', 'binary'],
				'conditions'  : [
					[ 'OS=="linux"', { 'action': ['mkdir', '-p', 'binary'] } ],
					[ 'OS=="mac"', { 'action': ['mkdir', '-p', 'binary'] } ],
					[ 'OS=="win"', { 'action': ['mkdir', 'binary'] } ],
				],
			}],
		},
		
		{
			'target_name'  : 'copy_binary',
			'type'         : 'none',
			'dependencies' : ['make_directory'],
			'actions'      : [{
				'action_name' : 'Module copied.',
				'inputs'      : [],
				'outputs'     : ['binary'],
				'conditions'  : [
					[ 'OS=="linux"', { 'action' : [
						'cp',
						'<(module_root_dir)/build/Release/glfw.node',
						'<(module_root_dir)/binary/glfw.node'
					] } ],
					[ 'OS=="mac"', { 'action' : [
						'echo', 'copy'
					] } ],
					[ 'OS=="win"', { 'action' : [
						'copy "<(module_root_dir)/build/Release/glfw.node"' +
						' "<(module_root_dir)/binary"'
					] } ],
				],
			}],
		},
		
		{
			'target_name'  : 'remove_extras',
			'type'         : 'none',
			'dependencies' : ['copy_binary'],
			'actions'      : [{
				'action_name' : 'Build intermediates removed.',
				'inputs'      : [],
				'outputs'     : ['src'],
				'conditions'  : [
					[ 'OS=="linux"', { 'action' : [
						'rm',
						'<(module_root_dir)/build/Release/obj.target/glfw/src/glfw.o',
						'<(module_root_dir)/build/Release/obj.target/glfw.node',
						'<(module_root_dir)/build/Release/glfw.node'
					] } ],
					[ 'OS=="mac"'  , { 'action' : [ 'echo', 'remove' ] } ],
					[ 'OS=="win"'  , { 'action' : [
						'<(module_root_dir)/_del "<(module_root_dir)/build/Release/glfw.*" && ' +
						'<(module_root_dir)/_del "<(module_root_dir)/build/Release/obj/glfw/*.*'
					] } ],
				],
			}],
		},
	]
}
