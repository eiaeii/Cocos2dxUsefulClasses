#include "FragmentEffect.h"

FragmentEffect* FragmentEffect::s_instance = 0;

FragmentEffect* FragmentEffect::getInstance()
{
	if(!s_instance)
		s_instance = new FragmentEffect();

	return s_instance;
}

void FragmentEffect::destroy()
{
	if(s_instance)
	{
		delete s_instance;
		s_instance = 0;
	}
}

FragmentEffect::FragmentEffect()
{
	m_nodeList.clear();

	//�����Ӽ���Ԥ����Ч������

	//û����Ч
	Matrix44 matrix0 = {
		1.0f, 0.0f, 0.0f, 0.0f,						
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
	m_matrices.push_back(matrix0);

	//�ڰ׻�
	Matrix44 matrix1 = {
		0.299f, 0.587f, 0.184f, 0.0f,
		0.299f, 0.587f, 0.184f, 0.0f,
		0.299f, 0.587f, 0.184f, 0.0f,
		0.0f,   0.0f,   0.0f,   1.0f
	};
	m_matrices.push_back(matrix1);

	//����Ƭ
	Matrix44 matrix2 = {
		0.299f, 0.587f, 0.184f, 0.3137f,
		0.299f, 0.587f, 0.184f, 0.1686f,
		0.299f, 0.587f, 0.184f,-0.0901f,
		0.0f,   0.0f,   0.0f,   1.0f
	};
	m_matrices.push_back(matrix2);

	//����
	Matrix44 matrix3 = {
	   -1.0f, 0.0f, 0.0f, 1.0f,
		0.0f,-1.0f, 0.0f, 1.0f,
		0.0f, 0.0f,-1.0f, 1.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
	m_matrices.push_back(matrix3);

	//���ˣ�ƫ�죩
	Matrix44 matrix4 = {
		1.0f,  0.0f, 0.0f, 0.0f,
		0.0f,  0.6f, 0.0f, 0.0f,
		0.0f,  0.0f, 0.6f, 0.0f,
		1.0f,  0.0f, 0.0f, 1.0f
	};
	m_matrices.push_back(matrix4);

	//�ж���ƫ�̣�
	Matrix44 matrix5 = {
		0.6f,  0.0f, 0.0f, 0.0f,
		0.0f,  1.0f, 0.0f, 0.0f,
		0.0f,  0.0f, 0.6f, 0.0f,
		0.0f,  1.0f, 0.0f, 1.0f
	};
	m_matrices.push_back(matrix5);

	//���䣨ƫ����
	Matrix44 matrix6 = {
		0.6f,  0.0f, 0.0f, 0.0f,
		0.0f,  0.6f, 0.0f, 0.0f,
		0.0f,  0.0f, 1.0f, 0.0f,
		0.0f,  0.0f, 1.0f, 1.0f
	};
	m_matrices.push_back(matrix6);
}

int FragmentEffect::addEffectNode(CCNode* pNode)
{
	do
	{
		CCGLProgram* program = new CCGLProgram();

		GLchar* pszFragSource =
				"#ifdef GL_ES													  \n \
				precision mediump float;										  \n \
				#endif															  \n \
				uniform sampler2D u_texture;									  \n \
				varying vec2 v_texCoord;										  \n \
				varying vec4 v_fragmentColor;									  \n \
				uniform mat4 matrixEffect;										  \n \
				void main(void)													  \n \
				{																  \n \
					gl_FragColor = texture2D(u_texture, v_texCoord)*matrixEffect; \n \
				}";

		program->initWithVertexShaderByteArray(ccPositionTextureColor_vert, //������ɫ��,�����������Դ���
												 pszFragSource);            //������ɫ��,�������Լ�д��

		//�����ɵ�shader���򴫸�CCSprite��
		pNode->setShaderProgram(program);
		
		//Ҫreleaseһ��,�������setShaderProgram��pProgram�йܸ�CCSprite
		//setShaderProgramֻ������pProgram->retain(),û��release(),����pProgram��m_uReference��2
		program->release();
		
		CHECK_GL_ERROR_DEBUG();
		
		//���ö�����ɫ����attribute���������ꡢ�������ꡢ��ɫ ,ȥ������һ�����������ܹ���
		program->addAttribute(kCCAttributeNamePosition, kCCVertexAttrib_Position);
		program->addAttribute(kCCAttributeNameColor,    kCCVertexAttrib_Color);
		program->addAttribute(kCCAttributeNameTexCoord, kCCVertexAttrib_TexCoords);
		
		CHECK_GL_ERROR_DEBUG();

		//�Զ�����ɫ������
		program->link();
		CHECK_GL_ERROR_DEBUG();

		// �����ƶ������š���ת����  
		program->updateUniforms();
		CHECK_GL_ERROR_DEBUG();

		EffectListNode listNode;
		listNode._pNode         = pNode;
		listNode._effectSelect  = 0;
		listNode._uniformMatrix = program->getUniformLocationForName("matrixEffect");

		program->use();
		program->setUniformLocationWithMatrix4fv(listNode._uniformMatrix,
												 m_matrices[0]._matrix,
												 1);
		m_nodeList.push_back(listNode);

		return m_nodeList.size()-1;

	} while (0);
	
	return -1;
}

bool FragmentEffect::setEffect(unsigned int nodeIndex,unsigned int sel)
{
	if(nodeIndex<0 || nodeIndex>=m_nodeList.size() || sel<0 || sel>=m_matrices.size())
		return false;

	CCGLProgram* program = m_nodeList[nodeIndex]._pNode->getShaderProgram();

	program->use();
	program->setUniformLocationWithMatrix4fv(m_nodeList[nodeIndex]._uniformMatrix,
											 m_matrices[sel]._matrix,
											 1);

	m_nodeList[nodeIndex]._effectSelect = sel;

	return true;
}

bool FragmentEffect::setEffect(CCNode* pNode,unsigned int sel)
{
	if(sel<0 || sel>=m_matrices.size())
		return false;

	std::vector<EffectListNode>::iterator i   = m_nodeList.begin();
	std::vector<EffectListNode>::iterator end = m_nodeList.end();

	while (i != end)
	{
		if(i->_pNode == pNode)
		{
			CCGLProgram* program = i->_pNode->getShaderProgram();

			program->use();
			program->setUniformLocationWithMatrix4fv(i->_uniformMatrix,
													 m_matrices[sel]._matrix,
													 1);

			i->_effectSelect = sel;

			return true;
		}

		i++;
	}

	return false;
}

int FragmentEffect::getEffectSelect(unsigned int nodeIndex)
{
	if(nodeIndex<0 || nodeIndex>=m_nodeList.size())
		return -1;
	else
		return m_nodeList[nodeIndex]._effectSelect;
}

int FragmentEffect::getEffectSelect(CCNode* pNode)
{
	std::vector<EffectListNode>::iterator i   = m_nodeList.begin();
	std::vector<EffectListNode>::iterator end = m_nodeList.end();

	while (i != end)
	{
		if(i->_pNode == pNode)
			return i->_effectSelect;

		i++;
	}

	return -1;
}