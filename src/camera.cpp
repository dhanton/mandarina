#include "camera.hpp"

#include "helper.hpp"

Camera::Camera()
{
    m_freeView = false;

    m_initialZoom = 0.7f; //was 0.8 before
    m_currentZoom = m_initialZoom;
}

void Camera::renderUpdate(sf::Time eTime)
{
#ifdef MANDARINA_DEBUG
    if (m_freeView) {
        Vector2 pos = m_view->getCenter();
        PlayerInput_applyInput(m_freeInput, pos, 500.f, eTime);
        m_view->setCenter(pos);

    } else {
#endif

    //smoothly move the camera to its next position

    m_timer += eTime;

    if (m_timer > m_currentTransition.time) {
        m_timer = m_currentTransition.time;
        m_view->setCenter(m_currentTransition.endPos);
    } else {
        m_view->setCenter(Helper_lerpVec2(m_prevPos, m_currentTransition.endPos, 
                            m_timer.asSeconds(), m_currentTransition.time.asSeconds()));
    }

#ifdef MANDARINA_DEBUG 
    }
#endif
}

void Camera::handleInput(const sf::Event& event)
{
#ifdef MANDARINA_DEBUG
    PlayerInput_handleInput(m_freeInput, event);

    //@TODO: interpolate to zoom smoothly
    if (event.type == sf::Event::MouseWheelScrolled) {
        float zoom = event.mouseWheelScroll.delta == -1.f ? 2.f : 1/2.f;
        m_currentZoom *= zoom;
        m_view->zoom(zoom);
    }
#endif
}

void Camera::changeState()
{
    m_freeView = !m_freeView;

    if (m_freeView) {
        m_currentTransition.time = sf::Time::Zero;

    } else {
        PlayerInput_clearKeys(m_freeInput);

        //reset the zoom
        m_view->zoom(m_initialZoom/m_currentZoom);
        m_currentZoom = m_initialZoom;
    }
}

void Camera::snapInstant(const Vector2& pos)
{
    const Vector2 clamped = _clamp(pos);

    m_view->setCenter(clamped);
    m_currentTransition.endPos = clamped;
    m_currentTransition.time = sf::Time::Zero;
}

void Camera::snapSmooth(const Vector2& pos, sf::Time time, bool forceFinish)
{
    if (m_currentTransition.forceFinish && m_timer < m_currentTransition.time) {
        return;
    }

    m_prevPos = m_view->getCenter();
    m_timer = sf::Time::Zero;
    m_currentTransition.endPos = _clamp(pos);
    m_currentTransition.time = time;
    m_currentTransition.forceFinish = forceFinish;
}

Vector2 Camera::mapPixelToCoords(const Vector2i& pixel) const
{
    Vector2 pos = static_cast<Vector2>(pixel);
    Vector2 viewPos = m_view->getCenter() - m_view->getSize()/2.f;

    //transform window coordinates to world coordinates
    pos *= m_currentZoom;
    pos += viewPos;

    return pos;
}

void Camera::setMapSize(const Vector2u& size)
{
    m_mapSize = size;
}

void Camera::setView(sf::View* view)
{
    m_view = view;
    m_currentTransition.endPos = view->getCenter();
}

float Camera::getZoom() const
{
    return m_currentZoom;
}

bool Camera::isFreeView() const
{
    return m_freeView;
}

Vector2 Camera::_clamp(const Vector2& pos) const
{
    Vector2 viewSize = m_view->getSize();
    Vector2 newPos;

    if (m_mapSize.x < viewSize.x) {
        newPos.x = m_mapSize.x/2.f;
    } else {
        newPos.x = Helper_clamp(pos.x, viewSize.x/2.f, (float) m_mapSize.x - viewSize.x/2.f);
    }

    if (m_mapSize.y < viewSize.y) {
        newPos.y = m_mapSize.y/2.f;
    } else {
        newPos.y = Helper_clamp(pos.y, viewSize.y/2.f, (float) m_mapSize.y - viewSize.y/2.f);
    }

    return newPos;
}
