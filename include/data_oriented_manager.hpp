#pragma once 

#include <stack>

class DataOrientedManager
{
public:
    DataOrientedManager()
    {
        m_lastId = -1;
        m_itemNumber = 0;
    }

    //Removed items are added to the stack
    void itemIdRemoved(int id)
    {
        m_itemNumber--;
        m_ids.push(id);
    }

    //Get an item from the stack or a new id if the stack is empty
    int getNewItemId()
    {
        if (m_ids.empty()) {
            m_itemNumber++;

            return ++m_lastId;

        } else {
            int result = m_ids.top();
            m_ids.pop();

            return result;
        }
    }

    //Number of items in the parent data structure
    int getItemNumber() const
    {
        return m_itemNumber;
    }

private:
    std::stack<int> m_ids;
    int m_lastId;
    int m_itemNumber;
};