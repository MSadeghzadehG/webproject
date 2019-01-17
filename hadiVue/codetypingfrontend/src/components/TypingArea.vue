<template>
    <div>
        <h1 class="d-1 text-center">Typing Test</h1>
        <hr>
        <div class="container">
            <div class="row">
                <div class="col-md-6 text-center">
                    <h2 class="display-4">Timer :</h2>
                    <h2 class="timer display-5">{{timer}}</h2>
                </div>
                <div class="col-md-6 text-center">
                    <button class="btn btn-primary" v-on:click="reset()">Reset</button>
                    <!--<h2 class="timer display-5">{{timer}}</h2>-->
                </div>
            </div>
            <div class="row">
                <!--v-html="outputHTML"-->
                <div v-if="this.isTyping" class="col-md-12 text-center" v-html="computedSpanizedOriginalText">
                    <h4>BIAH</h4>
                </div>
                <div v-else class="col-md-12 text-center">
                    <p class="text-justify" style="color:black !important;"
                       v-html="this.getSpanizeTagString(this.originalTestParagraph)">

                    </p>
                </div>
            </div>
            <hr>
            <div class="row">
                <div class="col-md-12 text-center">
                    <textarea class="form-control" id="typingArea" v-model="typedText"></textarea>
                </div>
            </div>
            <div class="d-hidden" v-html="this.spanizedTypedText">

            </div>
        </div>
    </div>
</template>

<script>
    export default {
        name: "TypingArea",
        data: function () {
            return {
                originalTestParagraph: 'Lorem ipsum dolor sit amet, consectetur adipiscing elit. In auctor pulvinar dolor, nec tempus sem iaculis non. Fusce sagittis vitae leo non varius. Nullam dapibus, lacus vel commodo varius, quam diam laoreet massa, dapibus euismod dolor nulla at justo. Class aptent taciti sociosqu ad litora torquent per conubia nostra, per inceptos himenaeos. Ut bibendum elit lacus, in feugiat odio viverra sed. Maecenas vestibulum mi nec aliquet tristique. In interdum euismod metus vel dapibus. Sed sodales magna et dictum imperdiet. Aenean magna odio, tempus quis consequat sed, feugiat ut elit. Morbi quis eleifend velit. Nunc venenatis nisi eget mi elementum, nec blandit odio maximus. Phasellus interdum mattis sodales.',
                typedText: '',
                typoIndex: -1,
                numOfWords: 0,
                preDefTimer: 20,
                timer: 20,
                isTyping: false,
                timerInterval: '',
                typoNumber: 0,
                spanizedTypedText: '',
                spanizedOriginalText: ''
            }
        },
        methods: {
            getSpanizeTagString: function (string) {
                let spanizeHtml = '';
                let counter = 0;
                for (let i = 0; i < string.length; i++) {
                    counter++;
                    if (counter === 120) {
                        spanizeHtml += '<br>';
                        counter = 0;
                    }
                    spanizeHtml += '<span id="' + i + '" class="" >' + string[i] + '</span>';

                }
                return spanizeHtml;
            },
            startTypingSpeed: function () {
                this.isTyping = true;
                this.startTimer();
            },
            endTypingSpeed: function () {
                clearInterval(this.timerInterval);
                this.isTyping = false;
                this.timer = 0;
                document.getElementById("typingArea").setAttribute("disabled", "true");
                // this.checkTypos();
            },
            startTimer: function () {
                this.timerInterval = setInterval(() => {
                    if (this.timer === 0) {
                        this.endTypingSpeed();
                        return
                    }
                    this.timer--;
                }, 1000)
            },
            reset: function () {
                clearInterval(this.timerInterval);
                this.isTyping = false;
                this.typoIndex = -1;
                this.typedText = '';
                this.timer = this.preDefTimer;
                this.typoNumber = 0;
                document.getElementById("typingArea").removeAttribute("disabled");
            },
        },
        watch: {
            typedText: function (value) {
                if (!this.isTyping) {
                    this.startTypingSpeed();
                }
                // let spanizedTypedText = this.getSpanizeTagString(value);
                // this.numOfWords = this.countWord(value, ' ');
            },
        },
        computed: {
            // outputHTML: function () {
            //     // window.console.log(this.typedText)
            //     let newHTML = '<p class="text-justify">';
            //     newHTML += '<span class="label label-success">';
            //
            //     for(let i = 0 ; i < this.typedText.length ; i++){
            //         if(this.typoIndex !== i){
            //             newHTML += this.originalTestParagraph.substr(0, this.typedText.length);
            //         }else{
            //             //ta inja doros boode residim yeki ke ghalate
            //             newHTML += '</span>';
            //             //ghalate ro beza intoo
            //             newHTML += '</span><span class="label label-danger>"' + this.originalTestParagraph.charAt(this.typoIndex);
            //         }
            //     }
            //     let characterToBeWrong = this.originalTestParagraph.charAt(this.typoIndex);
            //     if (this.typoIndex === -1) {
            //         newHTML += this.originalTestParagraph.substr(0, this.typedText.length) + '</span>';
            //         newHTML += this.originalTestParagraph.substr(this.typedText.length)
            //         newHTML += '</p>';
            //         return newHTML;
            //     }
            //     newHTML += this.originalTestParagraph.substr(0, this.typoIndex) + '</span>';
            //     newHTML += '<span class="label label-danger">';
            //     newHTML += this.originalTestParagraph.substring(this.typoIndex, this.typedText.length);
            //     newHTML += '</span>';
            //     newHTML += this.originalTestParagraph.substr(this.typedText.length)
            //     newHTML += '</p>';
            //     return newHTML;
            //
            // }
        },
        computedSpanizedOriginalText: function () {
            this.getSpanizeTagString(this.originalTestParagraph);
        },
        spanizedTypedText: function () {
            return this.getSpanizeTagString(this.typedText);
        },
    }
</script>

<style>
    p {
        font-size: 15px !important;
        line-height: 30px;
    }

    .label {
        border-radius: 0 !important;
        font-size: 15px !important;
        padding: 0 !important;
        text-transform: none;
        font-weight: normal;
    }

    .label-danger {
        border-radius: 0 !important;
    }

    .label-success {
        border-radius: 0 !important;
    }
</style>
