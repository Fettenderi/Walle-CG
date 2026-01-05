#ifndef POOL_H
#define POOL_H

#include <vector>
#include <memory>
#include <utility>

//tipo generico, per poter creare pool per oggetti di tipo diverso
template <typename T>

class ObjectPool {
private:
	//vettore in cui vengono inseriti gli oggetti
	std::vector<std::shared_ptr<T>> pool;
public:
	template <typename... Args>
	ObjectPool(int startingSize, Args&&... args) {
		for (int i = 0; i < startingSize; i++) {
			pool.push_back(std::make_shared<T>(std::forward<Args>(args)...));
			//printf("aggiunto un oggetto alla pool\n");
		}
	}

	//per prendere un oggetto dalla pool
	std::shared_ptr<T> getInstance() {
		//se sono finiti gli oggetti nella pool ne creo uno nuovo e ritorno quello
		if (pool.empty()) {
			return nullptr;
		}

		//altrimenti ne prendo uno dalla pool
		std::shared_ptr<T> instance = pool.back();
		pool.pop_back();
		//printf("sto prendendo un oggetto dalla pool\n");
		return instance;
	}

	void returnToPool(std::shared_ptr<T> instance) {
		pool.emplace_back(instance);
	}
};

#endif