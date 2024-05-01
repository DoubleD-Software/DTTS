import adapter from '@sveltejs/adapter-static';
import { vitePreprocess } from '@sveltejs/vite-plugin-svelte';

/** @type {import('@sveltejs/kit').Config} */
const config = {
	kit: {
		adapter: adapter({
			pages: '../data/website',
			assets: '../data/website',
			fallback: undefined,
			precompress: false,
			strict: false,
		})
	},
	preprocess: vitePreprocess()
};

export default config;
