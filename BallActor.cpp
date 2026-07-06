#include "BallActor.h"
#include "Scene.h"
#include "Game.h"
#include "PaddleActor.h"
#include "BreakoutScene.h"
#include "BlockActor.h"

BallActor::BallActor(Scene* scene, float radius, COLORREF color,
	const Vector2d& pos, const Vector2d& vel)
	: Actor2d(scene, pos, vel)
	, m_radius(radius)
	, m_color(color)
{
}

BallActor::~BallActor()
{
}

void BallActor::update(float deltaTime)
{
	if (isDead()) return;

	{
		BreakoutScene* scene = (BreakoutScene*)m_scene;
		State state = scene->getState();

		if (state == State::Ready)
		{
			PaddleActor* paddle = scene->getPaddle();
			m_pos = paddle->getPos() - UnitVecY2d * 30.0f;
			return;
		}
		else if (state == State::Clear || state == State::Over)
		{
			return;
		}
	}

	m_pos += m_vel * deltaTime;

	Game* game = m_scene->getGame();
	int width = game->getWidth();
	int height = game->getHeight();

	if (m_pos.x - m_radius < 0.0f)
	{
		m_pos.x = m_radius;
		m_vel.x *= -1.0f;
	}
	if (m_pos.x + m_radius > width)
	{
		m_pos.x = width - m_radius;
		m_vel.x *= -1.0f;
	}
	if (m_pos.y - m_radius < 0.0f)
	{
		m_pos.y = m_radius;
		m_vel.y *= -1.0f;
	}
	if (m_pos.y - m_radius > height)
	{
		setDead();
		return;
	}

	{
		BreakoutScene* bscene = (BreakoutScene*)m_scene;
		PaddleActor* paddle = bscene->getPaddle();
		Box rect = paddle->getBox();

		Segment left, right, top, bottom;
		separateBoxToSegments(rect, left, right, top, bottom);
		Circle c = getCircle();

		if (detectCircleToSegmentCollision(c, top))
		{
			// 上辺との衝突
			if (m_vel.y > 0.0f)
			{
				m_vel.y *= -1.0f;
				m_pos.y = rect.pos.y - m_radius;
			}
			else
			{
				if (detectCircleToSegmentCollision(c, left))
				{
					m_vel.x *= -1.0f;
					m_pos.x = rect.pos.x - m_radius;
				}
				else if (detectCircleToSegmentCollision(c, right))
				{
					m_vel.x *= -1.0f;
					m_pos.x = rect.pos.x + rect.width + m_radius;
				}
			}
		}
		else if (detectCircleToSegmentCollision(c, bottom))
		{
			// 下辺との衝突
			if (m_vel.y < 0.0f)
			{
				m_vel.y *= -1.0f;
				m_pos.y = rect.pos.y + rect.height + m_radius;
			}
			else
			{
				if (detectCircleToSegmentCollision(c, left))
				{
					m_vel.x *= -1.0f;
					m_pos.x = rect.pos.x - m_radius;
				}
				else if (detectCircleToSegmentCollision(c, right))
				{
					m_vel.x *= -1.0f;
					m_pos.x = rect.pos.x + rect.width + m_radius;
				}
			}
		}
		else if (detectCircleToSegmentCollision(c, left))
		{
			// 左辺との衝突
			if (m_vel.x > 0.0f)
			{
				m_vel.x *= -1.0f;
				m_pos.x = rect.pos.x - m_radius;
			}
		}
		else if (detectCircleToSegmentCollision(c, right))
		{
			// 右辺との衝突
			if (m_vel.x < 0.0f)
			{
				m_vel.x *= -1.0f;
				m_pos.x = rect.pos.x + rect.width + m_radius;
			}
		}
	}

	{
		// ボールとブロックの反射処理
		BreakoutScene* bscene = (BreakoutScene*)m_scene;
		std::vector<Actor*> blocks = bscene->getBlocks();
		for (auto it = blocks.begin(); it != blocks.end(); ++it)
		{
			BlockActor* block = (BlockActor*)(*it);
			if (block->isDead()) continue;

			Box rect = block->getBox();

			Segment left, right, top, bottom;
			separateBoxToSegments(rect, left, right, top, bottom);
			Circle c = getCircle();

			if (detectCircleToSegmentCollision(c, top))
			{
				// 上辺との衝突
				if (m_vel.y > 0.0f)
				{
					m_vel.y *= -1.0f;
					m_pos.y = rect.pos.y - m_radius;
					block->setDead();
				}
				else
				{
					if (detectCircleToSegmentCollision(c, left))
					{
						m_vel.x *= -1.0f;
						m_pos.x = rect.pos.x - m_radius;
						block->setDead();
					}
					else if (detectCircleToSegmentCollision(c, right))
					{
						m_vel.x *= -1.0f;
						m_pos.x = rect.pos.x + rect.width + m_radius;
						block->setDead();
					}
				}
			}
			else if (detectCircleToSegmentCollision(c, bottom))
			{
				// 下辺との衝突
				if (m_vel.y < 0.0f)
				{
					m_vel.y *= -1.0f;
					m_pos.y = rect.pos.y + rect.height + m_radius;
					block->setDead();
				}
				else
				{
					if (detectCircleToSegmentCollision(c, left))
					{
						m_vel.x *= -1.0f;
						m_pos.x = rect.pos.x - m_radius;
						block->setDead();
					}
					else if (detectCircleToSegmentCollision(c, right))
					{
						m_vel.x *= -1.0f;
						m_pos.x = rect.pos.x + rect.width + m_radius;
						block->setDead();
					}
				}
			}
			else if (detectCircleToSegmentCollision(c, left))
			{
				// 左辺との衝突
				if (m_vel.x > 0.0f)
				{
					m_vel.x *= -1.0f;
					m_pos.x = rect.pos.x - m_radius;
					block->setDead();
				}
			}
			else if (detectCircleToSegmentCollision(c, right))
			{
				// 右辺との衝突
				if (m_vel.x < 0.0f)
				{
					m_vel.x *= -1.0f;
					m_pos.x = rect.pos.x + rect.width + m_radius;
					block->setDead();
				}
			}

		}
	}
}

void BallActor::draw()
{
	BreakoutScene* scene = (BreakoutScene*)m_scene;
	State state = scene->getState();
	if (state == State::Clear || state == State::Over) return;

	m_scene->getGame()->drawCircle(getCircle(), m_color);
}