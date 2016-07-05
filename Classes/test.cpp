#include "test.h"
#include "start.h"
#include "RandomNum.h"
#include "win.h"
#include <vector>
#include <cmath>

#define database UserDefault::getInstance() // ���ش洢ʵ��
#define INIT_SPEED 250 // ʵ���ٶ�Ϊ��ʼ�ٶ�*���ʱ��
#define MAX_TOUCH_TIME 2.0f // �������ʱ�䣬��������ٶ�
#define PI 3.14159265 // Բ����
#define AI_SHOOT_TIME 5.0f // AI������
#define GIFT_SCORE 25 // ÿ��Ŀ��ķ���
#define G -200.0f // �������ٶ�
#define TARGET_SCORE 100 // Ŀ�����

// ����Ͷʯ����λ��
const Vec2 shootPosition = Vec2(120, 120);
const Vec2 AIshootPosition = Vec2(680, 120);

float tmpAIshootTime;
std::vector<Vec2> allGiftPos, AIAllGiftPos;
RandomNum random_num;

/*
��������������ĳ���
�����������ٶȴ�С����
*/
cocos2d::Scene * Test::createScene()
{
	auto scene = Scene::createWithPhysics();
	scene->getPhysicsWorld()->setGravity(Vec2(0, G));
	auto layer = Test::create(scene->getPhysicsWorld());
	scene->addChild(layer);
	return scene;
}

Test* Test::create(PhysicsWorld* pw)
{
	Test* pRet = new Test();
	if (pRet && pRet->init(pw)) {
		return pRet;
	}
	pRet = NULL;
	return NULL;
}

bool Test::init(PhysicsWorld* pw)
{
	if (!Layer::init())
	{
		return false;
	}
	
	// ��ʼ��giftλ���б�
	allGiftPos.clear();
	AIAllGiftPos.clear();

	allGiftPos.push_back(Vec2(750, 250));
	allGiftPos.push_back(Vec2(750, 350));
	allGiftPos.push_back(Vec2(750, 450));
	allGiftPos.push_back(Vec2(600, 450));

	AIAllGiftPos.push_back(Vec2(50, 250));
	AIAllGiftPos.push_back(Vec2(50, 350));
	AIAllGiftPos.push_back(Vec2(50, 450));
	AIAllGiftPos.push_back(Vec2(200, 450));

	// ��ʼ����Һ�AI��������������
	playScore = AIScore = 0;
	currentTime = startTime = 0;
	tmpAIshootTime = 0.0f;

	// cocos2dx��ʱ�� 
	schedule(schedule_selector(Test::updateTime), 0.1);

	// ��ȡ��ǰ���Ӵ��ڴ�С
	visibleSize = Director::getInstance()->getVisibleSize();

	// ��ӱ���ͼƬ
	auto background = Sprite::create("bg.jpg");
	background->setPosition(visibleSize.width / 2, visibleSize.height / 2);
	this->addChild(background, 0);

	// ��ӵ������
	Node* ground = Node::create();
	ground->setPhysicsBody(PhysicsBody::createEdgeSegment(Vec2(0, 40), Vec2(800, 40))); // ������һ������Ϊ�߽�
	ground->getPhysicsBody()->setDynamic(false); // ���þ�̬ʹ��λ�ò���
	ground->setTag(0); // ���ñ�ţ�������ײ�ж�
	ground->getPhysicsBody()->setContactTestBitmask(0xFFFFFFFF); // ������ײ�����ˣ�ȫ0Ϊ������ײ���
	this->addChild(ground, 1);
	
	// ����м䵲��
	auto brick = Sprite::create("brick.png");
	brick->setPosition(Vec2(visibleSize.width / 2, 90));
	brick->setPhysicsBody(PhysicsBody::createBox(Size(20, 100))); // ���һ�������εĸ���
	brick->getPhysicsBody()->setDynamic(false);
	brick->getPhysicsBody()->setContactTestBitmask(0xFFFFFFFF);
	brick->setTag(0);
	this->addChild(brick, 1);

	// Back��ť
	auto item = MenuItemLabel::create(Label::createWithTTF("Back", "fonts/Marker Felt.ttf", 36), CC_CALLBACK_1(Test::goBack, this));
	auto menu = Menu::create(item, NULL);
	menu->setPosition(item->getContentSize().width / 2, item->getContentSize().height / 2);
	menu->setColor(Color3B::BLACK);
	this->addChild(menu, 5);

	// ������Ͷʯ��
	shooter = Sprite::create("shooter.png");
	shooter->setAnchorPoint(Vec2(0, 0)); // ����ê��ΪSprite�����½�
	shooter->setPosition(100, 35);
	shooter->setPhysicsBody(PhysicsBody::createCircle(25.0f, PhysicsMaterial(), Vec2(20, -20))); // ���ø��壨��ƫ�ƣ�
	shooter->getPhysicsBody()->setDynamic(false);
	shooter->setTag(3);
	shooter->getPhysicsBody()->setContactTestBitmask(0xFFFFFFFF);
	this->addChild(shooter, 2);

	// ���AIͶʯ��
	AIshooter = Sprite::create("AIshooter.png");
	AIshooter->setAnchorPoint(Vec2(1, 0)); // ����ê��ΪSprite�����½�
	AIshooter->setPosition(700, 35);
	AIshooter->setPhysicsBody(PhysicsBody::createCircle(25.0f, PhysicsMaterial(), Vec2(-20, -20)));
	AIshooter->getPhysicsBody()->setDynamic(false);
	AIshooter->setTag(6);
	AIshooter->getPhysicsBody()->setContactTestBitmask(0xFFFFFFFF);
	this->addChild(AIshooter, 2);

	// ��Ӽ�ͷ������������ʾ
	arrow = Sprite::create("arrow.png");
	arrow->setAnchorPoint(Vec2(-0.5, 0.5)); // ����ê�㣬������ê��ת
	arrow->setPosition(shootPosition);
	arrow->setVisible(false); // ��ʼ��ʱ������
	this->addChild(arrow, 2);

	// �����ҵ�target
	for (auto pos : allGiftPos) {
		auto gift = Sprite::create("gift.png");
		gift->setPosition(pos);
		gift->setPhysicsBody(PhysicsBody::createCircle(50.0f));
		gift->getPhysicsBody()->setDynamic(false);
		gift->setTag(2);
		gift->getPhysicsBody()->setContactTestBitmask(0xFFFFFFFF);
		this->addChild(gift, 1);
	}

	// ���AI��target
	for (auto pos : AIAllGiftPos) {
		auto gift = Sprite::create("AIgift.png");
		gift->setPosition(pos);
		gift->setPhysicsBody(PhysicsBody::createCircle(50.0f));
		gift->getPhysicsBody()->setDynamic(false);
		gift->setTag(5);
		gift->getPhysicsBody()->setContactTestBitmask(0xFFFFFFFF);
		this->addChild(gift, 1);
	}

	// ��ʾĿ�����
	char c[30];
	sprintf(c, "Target:%d", TARGET_SCORE);
	auto targetLabel = Label::createWithTTF(c, "fonts/Marker Felt.ttf", 36);
	targetLabel->setPosition(visibleSize.width / 2, visibleSize.height - targetLabel->getContentSize().height / 2);
	targetLabel->setColor(Color3B::RED);
	this->addChild(targetLabel, 5);

	// ��ʾ˫������
	char t[30];
	sprintf(t, "Score:%d  AIScore:%d", playScore, AIScore);
	scoreLabel = Label::createWithTTF(t, "fonts/Marker Felt.ttf", 36);
	scoreLabel->setPosition(visibleSize.width / 2, visibleSize.height - scoreLabel->getContentSize().height * 3 / 2);
	scoreLabel->setColor(Color3B::BLACK);
	this->addChild(scoreLabel, 5);

	// �����¼�
	touchEvent();

	// ��ײ�¼�
	contactEvent();

	return true;
}

/*
���ذ�ť�ص�����
������л���start����
*/
void Test::goBack(Ref * ref)
{
	auto scene = Start::createScene();
	Director::getInstance()->replaceScene(scene);
}

/*
update����
ÿ��һ��dtִ��һ��
dt��λ����
*/
void Test::updateTime(float dt)
{
	visibleSize = Director::getInstance()->getVisibleSize();
	currentTime += dt;
	tmpAIshootTime += dt; // ��¼AI������
	// AI��ʱ����AI��������ִ�����
	if (tmpAIshootTime >= AI_SHOOT_TIME) {
		tmpAIshootTime = 0.0f; // AI��ʱ����
		AIshoot(AIselectTarget()); // AI���
	}
}

/*
��ȡͣ���ڵ�ǰҳ����ʱ��
*/
float Test::getTime()
{
	return currentTime;
}

/*
��¼�����ʼʱ��ʱ��
*/
void Test::setStartTime()
{
	startTime = getTime();
}

/*
����������ʱ��=��ǰʱ��-��ʼʱ��
*/
float Test::getTouchTime()
{
	return currentTime - startTime;
}

/*
������ҷ����ڵ��ķ�������
*/
Vec2 Test::getShootVelocity()
{
	float touchTime = this->getTouchTime();
	if (touchTime > MAX_TOUCH_TIME) touchTime = MAX_TOUCH_TIME;
	float a = mousePosition.x - shootPosition.x;
	float b = mousePosition.y - shootPosition.y;
	float c = sqrt(a * a + b * b);
	return Vec2(INIT_SPEED * touchTime / c * a, INIT_SPEED * touchTime / c * b);
}

/*
�����ͷ����ת�Ƕ�
*/
float Test::getArrowRotation()
{
	float a = mousePosition.x - shootPosition.x;
	float b = mousePosition.y - shootPosition.y;
	if (a < 0) return -180 / PI * atan(b / a) + 180;
	return -180 / PI * atan(b / a);
}

/*
��¼��ǰ�����λ��
*/
void Test::setMousePosition(Vec2 pos)
{
	mousePosition = pos;
}

/*
��ȡ��ͷʵ��
*/
Sprite * Test::getArrow()
{
	return arrow;
}

/*
���·������
�������һ���ﵽĿ�����������Ϸ����
��ת����������
*/
void Test::updateScore()
{
	char t[30];
	sprintf(t, "Score:%d  AIScore:%d", playScore, AIScore);
	scoreLabel->setString(t);
	if (playScore >= TARGET_SCORE || AIScore >= TARGET_SCORE) {
		if (playScore >= TARGET_SCORE) {
			recordUserDefault(true); // ʹ�ñ��ؼ�¼������Ϸ���
		}
		else {
			recordUserDefault(false); // ʹ�ñ��ؼ�¼������Ϸ���
		}
		gameOver(); // ��ת����������
	}
}

/*
�����¼�
���� ���ؿ�ʼ�������ƶ������ؽ��� �����ص�����
*/
void Test::touchEvent()
{
	// ����������
	EventListenerTouchOneByOne* listener = EventListenerTouchOneByOne::create();
	// ��仰�����ʲô��˼����
	listener->setSwallowTouches(true);
	
	// ���ʱ��ʾ��ͷ����¼�����ʼʱ��
	listener->onTouchBegan = [this](Touch* touch, Event* e) {
		this->setStartTime();
		this->setMousePosition(touch->getLocation());
		this->getArrow()->setRotation(this->getArrowRotation());
		this->getArrow()->setColor(Color3B(255, 255, 255));
		this->getArrow()->setVisible(true);
		return true;
	};

	// ����ƶ�ʱ��ת��ͷ�����ı��ͷ��ɫ
	// ������ƶ����ᴥ���ûص�������= =
	listener->onTouchMoved = [this](Touch* touch, Event* e) {
		this->setMousePosition(touch->getLocation());
		this->getArrow()->setRotation(this->getArrowRotation());
		float touchTime = this->getTouchTime();
		if (touchTime > 2) touchTime = 2;
		int temp = (int)(touchTime * 100);
		this->getArrow()->setColor(Color3B(255 - temp, 255 - temp, 255 - temp));

	};

	// ����������ؼ�ͷ
	// �����ڵ�ʵ��
	// ���ݵ������ʱ��ͷ��������ڵ��ķ����ٶ�����
	listener->onTouchEnded = [this](Touch* touch, Event* e) {

		this->getArrow()->setVisible(false);
		float touchTime = this->getTouchTime();

		this->setMousePosition(touch->getLocation());
		auto new_ball = Sprite::create("bullet.png");
		new_ball->setPhysicsBody(PhysicsBody::createCircle(20.0f));
		new_ball->getPhysicsBody()->setContactTestBitmask(0xFFFFFFFF);
		new_ball->setPosition(shootPosition);
		new_ball->getPhysicsBody()->setVelocity(this->getShootVelocity());
		new_ball->setTag(1);
		this->addChild(new_ball, 1);

		return true;
	};

	// �Ѵ��ؼ�������ӵ������¼�����
	Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, this);
}

/*
��ײ�¼�
*/
void Test::contactEvent()
{
	// ������ײ������
	auto listener = EventListenerPhysicsContact::create();

	// ��ײ����ʱ�Ļص�����
	listener->onContactBegin = [this](PhysicsContact& contact) {
		// ��ȡ������ײ����������A,B
		auto A = (Sprite*)contact.getShapeA()->getBody()->getNode();
		auto B = (Sprite*)contact.getShapeB()->getBody()->getNode();

		/*
		Tag 0 ���� ����
		Tag 1 ����ڵ�
		Tag 2 ���Ŀ��
		Tag 3 ���Ͷʯ��
		Tag 4 AI�ڵ�
		Tag 5 AIĿ��
		Tag 6 AIͶʯ��
		*/

		// �ڵ���������򵲰� ���ڵ���ʧ
		if ((A->getTag() == 1 && B->getTag() == 0) || (A->getTag() == 0 && B->getTag() == 1) ||
			(A->getTag() == 4 && B->getTag() == 0) || (A->getTag() == 0 && B->getTag() == 4))
		{
			if (A->getTag() == 0) {
				this->spriteFadeOut(B);
			}
			else {
				this->spriteFadeOut(A);
			}
			return true;
		}

		// �����ڵ���ײ ����ʧ
		// ������������ͬһ�ߵ��ڵ�
		if ((A->getTag() == 1 && B->getTag() == 4) || (A->getTag() == 4 && B->getTag() == 1) ||
			(A->getTag() == 1 && B->getTag() == 1) || (A->getTag() == 4 && B->getTag() == 4))
		{
			this->spriteFadeOut(A);
			this->spriteFadeOut(B);
			return true;
		}

		// ����ڵ�����AIĿ��������Ͷʯ�� ���ڵ���ʧ
		if ((A->getTag() == 1 && B->getTag() == 3) || (A->getTag() == 3 && B->getTag() == 1) ||
			(A->getTag() == 1 && B->getTag() == 5) || (A->getTag() == 5 && B->getTag() == 1))
		{
			if (A->getTag() == 1) {
				this->spriteFadeOut(A);
			}
			else {
				this->spriteFadeOut(B);
			}
			return true;
		}

		// ����ڵ��������Ŀ�� �ڵ���Ŀ�����ʧ ���ӷ���
		if ((A->getTag() == 1 && B->getTag() == 2) || (A->getTag() == 2 && B->getTag() == 1)) {
			Vec2 giftPosition;
			if (A->getTag() == 1) {
				giftPosition = B->getPosition();
			}
			else {
				giftPosition = A->getPosition();
			}
			
			this->spriteFadeOut(A);
			this->spriteFadeOut(B);
			this->showPerScore(giftPosition, GIFT_SCORE);
			this->addPlayScore(GIFT_SCORE);
			return true;
		}

		// AI�ڵ�����AIĿ�� �ڵ���Ŀ�궼��ʧ AI��������
		if ((A->getTag() == 4 && B->getTag() == 5) || (A->getTag() == 5 && B->getTag() == 4)) {
			Vec2 giftPosition;
			if (A->getTag() == 4) {
				giftPosition = B->getPosition();
			}
			else {
				giftPosition = A->getPosition();
			}
			
			// �ѱ����е�Ŀ��λ�ô�AIĿ���б���ɾ��
			for (std::vector<Vec2>::iterator it = AIAllGiftPos.begin(); it != AIAllGiftPos.end(); it++) {
				if ((*it) == giftPosition) {
					AIAllGiftPos.erase(it);
					break;
				}
			}

			this->spriteFadeOut(A);
			this->spriteFadeOut(B);
			this->showPerScore(giftPosition, GIFT_SCORE);
			this->addAIScore(GIFT_SCORE);
			return true;
		}

		// ����ڵ�����AIͶʯ�� AIͶʯ��ѣ�Σ�.δʵ��.��
		if ((A->getTag() == 1 && B->getTag() == 6) || (A->getTag() == 6 && B->getTag() == 1)) {
			if (A->getTag() == 1) {
				this->spriteFadeOut(A);
			}
			else {
				this->spriteFadeOut(B);
			}
			/*
			AI stop shoot 2 second
			*/
			return true;
		}

		// AI�ڵ��������Ͷʯ�� ���Ͷʯ��ѣ�Σ�.δʵ��.��
		if ((A->getTag() == 3 && B->getTag() == 4) || (A->getTag() == 4 && B->getTag() == 3)) {
			if (A->getTag() == 4) {
				this->spriteFadeOut(A);
			}
			else {
				this->spriteFadeOut(B);
			}
			/*
			play stop shoot 2 second
			*/
			return true;
		}

		// AI�ڵ��������Ŀ���AIͶʯ�� ���ڵ���ʧ
		if ((A->getTag() == 4 && B->getTag() == 2) || (A->getTag() == 2 && B->getTag() == 4) ||
			(A->getTag() == 4 && B->getTag() == 6) || (A->getTag() == 6 && B->getTag() == 4))
		{
			if (A->getTag() == 4) {
				this->spriteFadeOut(A);
			}
			else {
				this->spriteFadeOut(B);
			}
			return true;
		}

		return true;
	};

	// ����ײ��������ӵ������¼�����
	Director::getInstance()->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, this);
}

/*
�Ƴ�ĳ������
*/
void Test::spriteFadeOut(Sprite * sprite)
{
	// ������ײ�����  �����ڵ���ʱ����Ҳ�ܴ�����ײ�¼�
	sprite->getPhysicsBody()->setContactTestBitmask(0x00000000);
	// ���õ���ʱ��
	auto fadeOut = FadeOut::create(0.05f);
	// �þ���ִ��һ���¼����������Ƴ��� ���Ƴ�ֻ�Ǳ�͸������
	sprite->runAction(Sequence::create(fadeOut,
		CallFunc::create(CC_CALLBACK_0(Sprite::removeFromParent, sprite)), NULL));
}

/*
������ҷ���
*/
void Test::addPlayScore(int addScore)
{
	playScore += addScore;
	updateScore();
}

/*
����AI����
*/
void Test::addAIScore(int addScore)
{
	AIScore += addScore;
	updateScore();
}

/*
��Ϸ������ת����������
*/
void Test::gameOver()
{
	auto scene = Win::createScene();
	Director::getInstance()->replaceScene(scene);
}

/*
��ʾ���з���
�ȵ���󵭳�
*/
void Test::showPerScore(Vec2 pos, int score)
{
	char t[30];
	sprintf(t, "%d", score);
	auto label = Label::createWithTTF(t, "fonts/Marker Felt.ttf", 48);
	label->setColor(Color3B::RED);
	label->setPosition(pos);
	this->addChild(label, 2);
	auto fadeIn = FadeIn::create(0.2f);
	auto fadeOut = FadeOut::create(0.8f);
	label->runAction(Sequence::create(fadeIn, fadeOut,
		CallFunc::create(CC_CALLBACK_0(Sprite::removeFromParent, label)), NULL));
}

/*
AIѡ�񹥻�Ŀ�꣬��20%���ʹ������Ͷʯ��
*/
Vec2 Test::AIselectTarget()
{
	int random = random_num.getRandomNum(10);
	if (random < 2) return shooter->getPosition();
	return AIAllGiftPos[random_num.getRandomNum(AIAllGiftPos.size())];
}

/*
����AI�ڵ�������Ŀ��
û�������ƣ�������
*/
void Test::AIshoot(Vec2 targetPos)
{

	float t = sqrt(2 * (AIshootPosition.x + AIshootPosition.y - targetPos.x - targetPos.y) / -G);
	float vx = (AIshootPosition.x - targetPos.x) / t;
	float vy = vx;

	auto new_ball = Sprite::create("AIbullet.png");
	new_ball->setPhysicsBody(PhysicsBody::createCircle(20.0f));
	new_ball->getPhysicsBody()->setContactTestBitmask(0xFFFFFFFF);
	new_ball->setPosition(AIshootPosition);
	new_ball->getPhysicsBody()->setVelocity(Vec2(-vx, vy));
	new_ball->setTag(4);
	this->addChild(new_ball, 1);
}

/*
���ر�����Ϸ����
*/
void Test::recordUserDefault(bool isWin)
{
	database->setBoolForKey("isWin", isWin);
}
