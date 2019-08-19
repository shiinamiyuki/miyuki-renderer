#pragma once


namespace Miyuki {
	class Preprocessable {
	public:
		virtual void preprocess() = 0;
	};

	/* Caches preprocessed data
		Recomputes only when hasChanged()
	*/
	class CachedPreprocessable : public Preprocessable {
		std::atomic<bool> _changed = true;
	public:
		virtual void doPreprocess() = 0;
		void preprocess()override final {
			if (_changed) {
				doPreprocess();
				_changed = false;
			}
		}

		void notifyChange() {
			_changed = true;
		}
		bool hasChanged()const { return _changed; }
		void forcePreprocess() {
			doPreprocess();
		}
	};
}