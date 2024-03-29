#pragma once
#include <cstdint>
#include <exception>
#include <stdexcept>
#include <iostream>

template <typename Key,
	typename T,
	typename size_type, const size_type Size,
	class Compare = std::less<Key>,
	class Allocator = std::allocator<std::pair<const Key, T>>
	>
class avl_array
{
	// Класс указателя индекса потомка
	typedef struct tag_child_type {
		size_type left;
		size_type right;
	}
	child_type;

	// Хранилище узлов
	Key key_[Size];			// Ключ
	T val_[Size];			// Значение
	std::int8_t balance_[Size];	// Баланс поддерева
	child_type child_[Size];	// Потомки узла
	size_type parent_[Size];	// Предок узла
	size_type size_;		// Действительный размер
	size_type root_;		// Корневой узел

	// Значение недействительного индекса
	static const size_type EMPTY = Size;

	// Класс итератора
	typedef class iterator {
		avl_array* instance_; 	// Экземпляр массива
		size_type idx_;		// Текущий узел

		friend class reverse_iterator;
		friend avl_array; // Массиву avl_array нужен доступ к индексу указателей
	public:
		// Конструктор
		iterator(avl_array* instance = nullptr, size_type idx = 0U)
			: instance_(instance), idx_(idx) {}

		bool operator==(const iterator& rhs) const
		{
			return idx_ == rhs.idx_;
		}

		bool operator!=(const iterator& rhs) const
		{
			return !(*this == rhs);
		}

		// Доступ к значению
		T& operator*() const
		{
			return val();
		}

		// Метод доступа к значению
		T& val() const
		{
			return instance_->val_[idx_];
		}

		// Доступ к значению ключа
		Key& key() const
		{
			return instance_->key_[idx_];
		}

		// Преинкремент
		iterator& operator++()
		{
			// Достигнут конец?
			if (idx_ >= Size) {
				return *this;
			}

			// Самый левый потомок правого потомка. При отсутствии - берём предка
			size_type i = instance_->child_[idx_].right;
			if (i != instance_->EMPTY) {
				// Преемник - самый дальний левый узел правого поддерева
				for (; i != instance_->EMPTY; i = instance_->child_[i].left) {
					idx_ = i;
				}
			}
			else {
				// Левое поддерево обработано и правого поддерева нет.
				// Перемещаемся вверх по дереву и ищем предка левого потомка,
				// останавливаем поиск когда предок становится NULL. Не NULL
				// предок - преемник. Если предок NULL, оригинальный узел был 
				// крайним и преемник - это конец списка.
				i = instance_->get_parent(idx_);
				while ((i != instance_->EMPTY) && (idx_ == instance_->child_[i].right)) {
					idx_ = i;
					i = instance_->get_parent(idx_);
				}
				idx_ = i;
			}
			return *this;
		}

		// Постинкремент
		iterator operator++(int)
		{
			auto _copy = *this;
			++(*this);
			return _copy;
		}
	}
	avl_array_iterator;

	// Класс реверсивного итератора
	typedef class reverse_iterator: public iterator {
		avl_array* instance_;	// Экземпляр массива
		size_type idx_;		// Действительный узел

		friend avl_array;	// Массиву avl_array нужен доступ к индексу указателей
	public:
		// Конструктор
		reverse_iterator(avl_array* instance = nullptr, size_type idx = 0U):
			instance_(instance), idx_(idx) {}
		reverse_iterator(const iterator& iter)
				: reverse_iterator(iter.instance_, iter.idx_)
		{
			++*this;
		}

		bool operator==(const reverse_iterator& rhs) const
		{
			return idx_ == rhs.idx_;
		}

		bool operator!=(const reverse_iterator& rhs) const
		{
			return !(*this == rhs);
		}

		// Доступ к значению
		T& operator*() const
		{
			return val();
		}

		// Метод доступа к значению
		T& val() const
		{
			return instance_->val_[idx_];
		}

		// Доступ к значению ключа
		Key& key() const
		{
			return instance_->key_[idx_];
		}

		// Преинкремент
		reverse_iterator& operator++()
		{
			// Достигнут конец?
			if( *this == nullptr)
				return *this;
			size_type i = instance_->child_[idx_].left;
			if (i != instance_->EMPTY) {
				for (; i != instance_->EMPTY; i = instance_->child_[i].right) {
					idx_ = i;
				}
			}
			else {
				i = instance_->get_parent(idx_);
				while ((i != instance_->EMPTY) && (idx_ == instance_->child_[i].left)) {
					idx_ = i;
					i = instance_->get_parent(idx_);
				}
				idx_ = i;
			}
			return *this;
		}

		// Постинкремент
		reverse_iterator operator++(int)
		{
			auto _copy = *this;
			++(*this);
			return _copy;
		}
	}
	avl_array_reverse_iterator;
public:
	typedef T value_type;
	typedef T* pointer;
	typedef const T* const_pointer;
	typedef T& reference;
	typedef const T& const_reference;
	typedef Key key_type;
	typedef avl_array_iterator iterator;
	typedef avl_array_reverse_iterator reverse_iterator;

	// Констуруктор
	avl_array(): size_(0U), root_(Size) {}

	// Итераторы
	iterator begin()
	{
		size_type i = EMPTY;
		if (root_ != EMPTY) {
			// Минимальный элемент - это дальний левый потомок корневого узла
			for (i = root_; child_[i].left != EMPTY; i = child_[i].left)
				;
		}
		return iterator(this, i);
	}

	iterator end()
	{
		return iterator(this, EMPTY);
	}

	reverse_iterator rend()
	{
		return reverse_iterator(begin());
	}

	reverse_iterator rbegin()
	{
		size_type i = EMPTY;
		if (root_ != EMPTY) {
			// Максимальный элемент - это дальний правый потомок корневого узла
			for (i = root_; child_[i].right != EMPTY; i = child_[i].right)
				;
		}
		return reverse_iterator(this, i);

	}

	// Текущий размер
	size_type size() const
	{
		return size_;
	}

	bool empty() const
	{
		return size_ == static_cast<size_type>(0);
	}

	// Ёмкость массива
	size_type capacity() const
	{
		return Size;
	}

	// Очистка контейнера
	void clear()
	{
		size_ = 0U;
		root_ = EMPTY;
	}

	// Вставка/обновление элемена. Если ключ существует - обновляется значение,
	// если не существует - узел булет добавлен.
	bool insert(const key_type& key, const value_type& val)
	{
		if (root_ == EMPTY) {
			key_[size_] = key;
			val_[size_] = val;
			balance_[size_] = 0;
			child_[size_] = { EMPTY, EMPTY };

			set_parent(size_, EMPTY);
			root_ = size_++;
			return true;
		}

		for (size_type i = root_; i != EMPTY; i = (key < key_[i]) ? child_[i].left : child_[i].right) {
			if (key < key_[i]) {
				if (child_[i].left == EMPTY) {
					if (size_ >= capacity()) {
						// Контейнер полон
						return false;
					}

					key_[size_] = key;
					val_[size_] = val;
					balance_[size_] = 0;
					child_[size_] = { EMPTY, EMPTY };

					set_parent(size_, i);
					child_[i].left = size_++;
					insert_balance(i, 1);
					return true;
				}
			}
			else if (key_[i] == key) {
				// Нашли ключ, обновляем значение
				val_[i] = val;
				return true;
			}
			else {
				if (child_[i].right == EMPTY) {
					if (size_ >= capacity()) {
						// Контейнер полон
						return false;
					}

					key_[size_] = key;
					val_[size_] = val;
					balance_[size_] = 0;
					child_[size_] = { EMPTY, EMPTY };

					set_parent(size_, i);
					child_[i].right = size_++;
					insert_balance(i, -1);
					return true;
				}
			}
		}

		// Узел не подходит = отбрасываем
		return false;
	}

	// Ищем элемент по ключу и обновляем значение. Возвращаем Truе или False.
	bool contains(const key_type& key, value_type& val) const
	{
		for (size_type i = root_; i != EMPTY;) {
			// Key found
			if (key == key_[i]) {
				val = val_[i];
				return true;
			}
			else if (key < key_[i]) {
				i = child_[i].left;
			}
			else {
				i = child_[i].right;
			}
		}
		// Ключ не найден
		return false;
	}

	// Ищем элемент и возвращаем итератор в качестве результата или end(), если он не найден
	iterator find(const key_type& key)
	{
		for (size_type i = root_; i != EMPTY;) {
			// Key found
			if (key == key_[i]) {
				return iterator(this, i);
			}
			else if (key < key_[i]) {
				i = child_[i].left;
			}
			else {
				i = child_[i].right;
			}
		}
		// Ключ не найден
		return end();
	}

	iterator at(const key_type& key)
	{
		for (size_type i = root_; i != EMPTY;) {
			if (key == key_[i]) {
				// found key
				return iterator(this, i);
			}
			else if (key < key_[i]) {
				i = child_[i].left;
			}
			else {
				i = child_[i].right;
			}
		}
		// Ключ не найден, возвращаем end()
		throw std::out_of_range("avl_tree[] out-of-range");
	}

	bool erase(const key_type& key)
	{
		return erase(find(key));
	}

	bool erase(iterator position)
	{
		if (empty() || (position == end())) {
			return false;
		}

		const size_type node = position.idx_;
		const size_type left = child_[node].left;
		const size_type right = child_[node].right;

		if (left == EMPTY) {
			if (right == EMPTY) {
				const size_type parent = get_parent(node);
				if (parent != EMPTY) {
					if (child_[parent].left == node) {
						child_[parent].left = EMPTY;
						delete_balance(parent, -1);
					}
					else {
						child_[parent].right = EMPTY;
						delete_balance(parent, 1);
					}
				}
				else {
					root_ = EMPTY;
				}
			}
			else {
				const size_type parent = get_parent(node);
				if (parent != EMPTY) {
					child_[parent].left == node ? child_[parent].left = right : child_[parent].right = right;
				}
				else {
					root_ = right;
				}

				set_parent(right, parent);
				delete_balance(right, 0);
			}
		}
		else if (right == EMPTY) {
			const size_type parent = get_parent(node);
			if (parent != EMPTY) {
				child_[parent].left == node ? child_[parent].left = left : child_[parent].right = left;
			}
			else {
				root_ = left;
			}

			set_parent(left, parent);
			delete_balance(left, 0);
		}
		else {
			size_type successor = right;
			if (child_[successor].left == EMPTY) {
				const size_type parent = get_parent(node);
				child_[successor].left = left;
				balance_[successor] = balance_[node];
				set_parent(successor, parent);
				set_parent(left, successor);

				if (node == root_) {
					root_ = successor;
				}
				else {
					if (child_[parent].left == node) {
						child_[parent].left = successor;
					}
					else {
						child_[parent].right = successor;
					}
				}

				delete_balance(successor, 1);
			}
			else {
				while (child_[successor].left != EMPTY) {
					successor = child_[successor].left;
				}

				const size_type parent = get_parent(node);
				const size_type successor_parent = get_parent(successor);
				const size_type successor_right = child_[successor].right;

				if (child_[successor_parent].left == successor) {
					child_[successor_parent].left = successor_right;
				}
				else {
					child_[successor_parent].right = successor_right;
				}

				set_parent(successor_right, successor_parent);
				set_parent(successor, parent);
				set_parent(right, successor);
				set_parent(left, successor);
				child_[successor].left = left;
				child_[successor].right = right;
				balance_[successor] = balance_[node];

				if (node == root_) {
					root_ = successor;
				}
				else {
					if (child_[parent].left == node) {
						child_[parent].left = successor;
					}
					else {
						child_[parent].right = successor;
					}
				}

				delete_balance(successor_parent, -1);
			}
		}

		size_--;

		// Переносим узел в конец узла, который был удалён, если он не удалён
		if (node != size_) {
			size_type parent = EMPTY;
			if (root_ == size_) {
				root_ = node;
			}
			else {
				parent = get_parent(size_);
				child_[parent].left == size_ ? child_[parent].left = node : child_[parent].right = node;
			}

			// Исправляем потомков родителя
			set_parent(child_[size_].left, node);
			set_parent(child_[size_].right, node);

			// Переносим содержимое
			key_[node] = key_[size_];
			val_[node] = val_[size_];
			balance_[node] = balance_[size_];
			child_[node] = child_[size_];
			set_parent(node, parent);
		}
		return true;
	}

	// Самопроверка целостности
	bool check() const
	{
		// Тест корня
		if (empty() && (root_ != EMPTY)) {
			// Неверный корень
			return false;
		}

		if (size() && root_ >= size()) {
			// Корень за пределами границ
			return false;
		}

		// Проверка дерева
		for (size_type i = 0U; i < size(); ++i) {
			if ((child_[i].left != EMPTY) && (key_[child_[i].left] >= key_[i])) {
				// Неверный ключ у левого потомка
				return false;
			}

			if ((child_[i].right != EMPTY) && (key_[child_[i].right] <= key_[i])) {
				// Неверный ключ у правого потомка
				return false;
			}

			const size_type parent = get_parent(i);
			if ((i != root_) && (parent == EMPTY)) {
				// Предок отсутствует
				return false;
			}

			if ((i == root_) && (parent != EMPTY)) {
				// Неверный предок корневого узла
				return false;
			}
		}
		// Тест пройден
		return true;
	}
	T& operator[](size_type key) {
	int a;

		if(!contains(key, a))
			insert(key, 0);
		return *at(key);
	}

private:
	// Поиск предка
	size_type get_parent(size_type node) const
	{
		return parent_[node];
	}

	// Установка указателя на предка
	void set_parent(size_type node, size_type parent)
	{
		if (node != EMPTY) {
			parent_[node] = parent;
		}
	}

	void insert_balance(size_type node, std::int8_t balance)
	{
		while (node != EMPTY) {
			balance = (balance_[node] += balance);

			if (balance == 0) {
				return;
			}
			else if (balance == 2) {
				if (balance_[child_[node].left] == 1) {
					rotate_right(node);
				}
				else {
					rotate_left_right(node);
				}

				return;
			}
			else if (balance == -2) {
				if (balance_[child_[node].right] == -1) {
					rotate_left(node);
				}
				else {
					rotate_right_left(node);
				}

				return;
			}

			const size_type parent = get_parent(node);
			if (parent != EMPTY) {
				balance = child_[parent].left == node ? 1 : -1;
			}

			node = parent;
		}
	}

	void delete_balance(size_type node, std::int8_t balance)
	{
		while (node != EMPTY) {
			balance = (balance_[node] += balance);

			if (balance == -2) {
				if (balance_[child_[node].right] <= 0) {
					node = rotate_left(node);
					if (balance_[node] == 1) {
						return;
					}
				}
				else {
					node = rotate_right_left(node);
				}
			}
			else if (balance == 2) {
				if (balance_[child_[node].left] >= 0) {
					node = rotate_right(node);
					if (balance_[node] == -1) {
						return;
					}
				}
				else {
					node = rotate_left_right(node);
				}
			}
			else if (balance != 0) {
				return;
			}

			if (node != EMPTY) {
				const size_type parent = get_parent(node);
				if (parent != EMPTY) {
					balance = child_[parent].left == node ? -1 : 1;
				}

				node = parent;
			}
		}
	}

	// 4 вида вращения дерева
	size_type rotate_left(size_type node)
	{
		const size_type right = child_[node].right;
		const size_type right_left = child_[right].left;
		const size_type parent = get_parent(node);

		set_parent(right, parent);
		set_parent(node, right);
		set_parent(right_left, node);
		child_[right].left = node;
		child_[node].right = right_left;

		if (node == root_) {
			root_ = right;
		}
		else if (child_[parent].right == node) {
			child_[parent].right = right;
		}
		else {
			child_[parent].left = right;
		}

		balance_[right]++;
		balance_[node] = -balance_[right];

		return right;
	}

	size_type rotate_right(size_type node)
	{
		const size_type left = child_[node].left;
		const size_type left_right = child_[left].right;
		const size_type parent = get_parent(node);

		set_parent(left, parent);
		set_parent(node, left);
		set_parent(left_right, node);
		child_[left].right = node;
		child_[node].left = left_right;

		if (node == root_) {
			root_ = left;
		}
		else if (child_[parent].left == node) {
			child_[parent].left = left;
		}
		else {
			child_[parent].right = left;
		}

		balance_[left]--;
		balance_[node] = -balance_[left];

		return left;
	}

	size_type rotate_left_right(size_type node)
	{
		const size_type left = child_[node].left;
		const size_type left_right = child_[left].right;
		const size_type left_right_right = child_[left_right].right;
		const size_type left_right_left = child_[left_right].left;
		const size_type parent = get_parent(node);

		set_parent(left_right, parent);
		set_parent(left, left_right);
		set_parent(node, left_right);
		set_parent(left_right_right, node);
		set_parent(left_right_left, left);
		child_[node].left = left_right_right;
		child_[left].right = left_right_left;
		child_[left_right].left = left;
		child_[left_right].right = node;

		if (node == root_) {
			root_ = left_right;
		}
		else if (child_[parent].left == node) {
			child_[parent].left = left_right;
		}
		else {
			child_[parent].right = left_right;
		}

		if (balance_[left_right] == 0) {
			balance_[node] = 0;
			balance_[left] = 0;
		}
		else if (balance_[left_right] == -1) {
			balance_[node] = 0;
			balance_[left] = 1;
		}
		else {
			balance_[node] = -1;
			balance_[left] = 0;
		}

		balance_[left_right] = 0;

		return left_right;
	}

	size_type rotate_right_left(size_type node)
	{
		const size_type right = child_[node].right;
		const size_type right_left = child_[right].left;
		const size_type right_left_left = child_[right_left].left;
		const size_type right_left_right = child_[right_left].right;
		const size_type parent = get_parent(node);

		set_parent(right_left, parent);
		set_parent(right, right_left);
		set_parent(node, right_left);
		set_parent(right_left_left, node);
		set_parent(right_left_right, right);
		child_[node].right = right_left_left;
		child_[right].left = right_left_right;
		child_[right_left].right = right;
		child_[right_left].left = node;

		if (node == root_) {
			root_ = right_left;
		}
		else if (child_[parent].right == node) {
			child_[parent].right = right_left;
		}
		else {
			child_[parent].left = right_left;
		}

		if (balance_[right_left] == 0) {
			balance_[node] = 0;
			balance_[right] = 0;
		}
		else if (balance_[right_left] == 1) {
			balance_[node] = 0;
			balance_[right] = -1;
		}
		else {
			balance_[node] = 1;
			balance_[right] = 0;
		}
		balance_[right_left] = 0;

		return right_left;
	}
};
